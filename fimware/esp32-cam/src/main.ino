#include <Arduino.h>
#include <stdlib.h>
#include <string.h>

#include "camera/camera_service.h"
#include "config/config_store.h"
#include "config/serial_cli.h"
#include "debug/logger.h"
#include "motor/roll_detector.h"
#include "network/mqtt_service.h"
#include "network/stream_service.h"
#include "network/webhook_client.h"
#include "network/wifi_service.h"
#include "system/app_config.h"

namespace {

AppConfig gConfig;
RollDetector gRoll;
uint32_t gLastEventMs = 0;
uint32_t gLastCameraRetryMs = 0;

String gMqttDeviceId;
String gMqttLogsTopic;
String gMqttConfigTopicWildcard;
String gMqttConfigWebhookTopic;
String gMqttConfigWifiTopic;
String gMqttConfigLedPowerTopic;
String gMqttConfigLedIntensityTopic;
String gMqttCommandsTopicWildcard;
String gMqttCommandCapturePhotoTopic;
String gMqttCommandStreamTopic;

String buildDeviceId() {
  uint64_t chipId = ESP.getEfuseMac();
  char id[13];
  snprintf(id, sizeof(id), "%012llX", static_cast<unsigned long long>(chipId & 0xFFFFFFFFFFFFULL));
  return String(id);
}

void setupMqttTopics() {
  gMqttDeviceId = buildDeviceId();

  String deviceRoot = String("devices/") + gMqttDeviceId;
  String configRoot = deviceRoot + "/config";
  String commandsRoot = deviceRoot + "/commands";

  gMqttLogsTopic = deviceRoot + "/logs";
  gMqttConfigTopicWildcard = configRoot + "/#";
  gMqttConfigWebhookTopic = configRoot + "/webhook";
  gMqttConfigWifiTopic = configRoot + "/wifi";
  gMqttConfigLedPowerTopic = configRoot + "/led/power";
  gMqttConfigLedIntensityTopic = configRoot + "/led/intensity";
  gMqttCommandsTopicWildcard = commandsRoot + "/#";
  gMqttCommandCapturePhotoTopic = commandsRoot + "/capture-photo";
  gMqttCommandStreamTopic = commandsRoot + "/stream";
}

bool parseBoolPayload(const char *payload, bool &value) {
  if (payload == nullptr) {
    return false;
  }

  String raw(payload);
  raw.trim();
  if (raw.length() == 0) {
    return false;
  }

  raw.toLowerCase();
  if (raw == "1" || raw == "on" || raw == "true") {
    value = true;
    return true;
  }

  if (raw == "0" || raw == "off" || raw == "false") {
    value = false;
    return true;
  }

  return false;
}

bool parseUInt8Payload(const char *payload, uint8_t &value) {
  if (payload == nullptr) {
    return false;
  }

  String raw(payload);
  raw.trim();
  if (raw.length() == 0) {
    return false;
  }

  char *endPtr = nullptr;
  long parsed = strtol(raw.c_str(), &endPtr, 10);
  if (endPtr == nullptr || *endPtr != '\0' || parsed < 0 || parsed > 255) {
    return false;
  }

  value = static_cast<uint8_t>(parsed);
  return true;
}

bool parseTextPayload(const char *payload, String &value, bool allowEmpty) {
  if (payload == nullptr) {
    return false;
  }

  String raw(payload);
  raw.trim();
  if (!allowEmpty && raw.length() == 0) {
    return false;
  }

  value = raw;
  return true;
}

bool parseWifiPayload(const char *payload, String &ssid, String &password) {
  if (payload == nullptr) {
    return false;
  }

  String raw(payload);
  raw.trim();
  if (raw.length() == 0) {
    return false;
  }

  int separator = raw.indexOf('|');
  if (separator < 0) {
    return false;
  }

  String nextSsid = raw.substring(0, separator);
  String nextPassword = raw.substring(separator + 1);
  nextSsid.trim();
  nextPassword.trim();

  if (nextSsid.length() == 0) {
    return false;
  }

  ssid = nextSsid;
  password = nextPassword;
  return true;
}

bool isCaptureCommandPayload(const char *payload) {
  if (payload == nullptr) {
    return false;
  }

  String raw(payload);
  raw.trim();
  if (raw.length() == 0) {
    return true;
  }

  raw.toLowerCase();
  return raw == "1" || raw == "on" || raw == "true" || raw == "capture" || raw == "now";
}

bool persistLedConfig() {
  app::Status saveSt = config_store::save(gConfig);
  if (saveSt != app::Status::kOk) {
    LOGE("Falha ao salvar config de LED");
    return false;
  }

  camera_service::applyLedState(false, 0);
  return true;
}

bool restorePreviousWifiConfig(const String &previousSsid, const String &previousPassword) {
  gConfig.ssid = previousSsid;
  gConfig.password = previousPassword;
  app::Status rollbackSt = config_store::save(gConfig);
  if (rollbackSt != app::Status::kOk) {
    LOGE("Rollback de WiFi falhou ao salvar config anterior");
    return false;
  }

  app::Status reconnectSt = wifi_service::connect(gConfig);
  if (reconnectSt != app::Status::kOk) {
    LOGE("Rollback de WiFi falhou ao reconectar rede anterior");
    return false;
  }

  LOGW("WiFi restaurado para configuracao anterior");
  return true;
}

bool tryStartCamera() {
  app::Status st = camera_service::init();
  if (st == app::Status::kOk) {
    camera_service::applyLedState(false, 0);
    return true;
  }

  gLastCameraRetryMs = millis();
  return false;
}

void applyWifiConfigWithRollback(const String &newSsid, const String &newPassword) {
  const String previousSsid = gConfig.ssid;
  const String previousPassword = gConfig.password;

  app::Status saveSt = config_store::setWifi(newSsid, newPassword, gConfig);
  if (saveSt != app::Status::kOk) {
    LOGW("MQTT WiFi rejeitado");
    return;
  }

  app::Status connectSt = wifi_service::connect(gConfig);
  if (connectSt == app::Status::kOk) {
    LOGI("MQTT WiFi atualizado e conectado");
    return;
  }

  LOGW("MQTT WiFi falhou ao conectar, executando rollback");
  restorePreviousWifiConfig(previousSsid, previousPassword);
}

void sendRollEvent(const RollMetrics &metrics) {
  app::Status wifiSt = wifi_service::ensureConnected(gConfig);
  if (wifiSt != app::Status::kOk) {
    LOGE("Sem WiFi para enviar evento");
    return;
  }

  if (!camera_service::isReady()) {
    LOGW("Camera indisponivel, tentando inicializar antes do evento");
    tryStartCamera();
  }

  const bool cameraReady = camera_service::isReady();
  const bool flashEnabled = cameraReady && gConfig.ledPower && gConfig.ledIntensity > 0;
  if (flashEnabled) {
    camera_service::applyLedState(true, gConfig.ledIntensity);
    delay(250);
  }

  camera_fb_t *fb = cameraReady ? camera_service::capture() : nullptr;

  // Desliga o LED imediatamente apos a captura para evitar ficar aceso
  // durante tentativas longas de upload/rede.
  if (flashEnabled) {
    camera_service::applyLedState(false, 0);
  }

  app::Status sendSt = app::Status::kSendError;
  if (fb != nullptr) {
    LOGI("Foto capturada: %u bytes", static_cast<unsigned>(fb->len));
    sendSt = webhook_client::sendPhotoMultipart(gConfig.webhookUrl, fb, metrics);
    camera_service::release(fb);
  } else {
    LOGW("Falha ao capturar frame");
  }

  if (sendSt == app::Status::kOk) {
    LOGI("Evento enviado com foto");
    return;
  }

  LOGW("Falha no multipart, usando fallback JSON");
  app::Status fallbackSt = webhook_client::sendJsonFallback(gConfig.webhookUrl, metrics);
  if (fallbackSt != app::Status::kOk) {
    LOGE("Fallback JSON falhou");
  }
}

void sendManualPhoto() {
  RollMetrics manualMetrics{};
  manualMetrics.pulses = 0;
  manualMetrics.rollTimeUs = 0;
  manualMetrics.entropy = 0;
  manualMetrics.dice = 0;
  sendRollEvent(manualMetrics);
}

void handleMqttMessage(const char *topic, const char *payload) {
  if (topic == nullptr || payload == nullptr) {
    return;
  }

  if (strcmp(topic, gMqttConfigWebhookTopic.c_str()) == 0) {
    String url;
    if (!parseTextPayload(payload, url, false)) {
      LOGW("MQTT webhook invalido: %s", payload);
      return;
    }

    app::Status st = config_store::setWebhook(url, gConfig);
    if (st != app::Status::kOk) {
      LOGW("MQTT webhook rejeitado");
      return;
    }

    LOGI("MQTT webhook atualizado");
    return;
  }

  if (strcmp(topic, gMqttConfigWifiTopic.c_str()) == 0) {
    String ssid;
    String password;
    if (!parseWifiPayload(payload, ssid, password)) {
      LOGW("MQTT WiFi invalido. Use <ssid>|<password>");
      return;
    }

    applyWifiConfigWithRollback(ssid, password);
    return;
  }

  if (strcmp(topic, gMqttConfigLedPowerTopic.c_str()) == 0) {
    bool power = false;
    if (!parseBoolPayload(payload, power)) {
      LOGW("MQTT LED power invalido: %s", payload);
      return;
    }

    gConfig.ledPower = power;
    if (!persistLedConfig()) {
      return;
    }
    LOGI("MQTT LED power atualizado: %s (modo flash)", gConfig.ledPower ? "ON" : "OFF");
    return;
  }

  if (strcmp(topic, gMqttConfigLedIntensityTopic.c_str()) == 0) {
    uint8_t intensity = 0;
    if (!parseUInt8Payload(payload, intensity)) {
      LOGW("MQTT LED intensity invalido: %s", payload);
      return;
    }

    gConfig.ledIntensity = intensity;
    if (!persistLedConfig()) {
      return;
    }
    LOGI("MQTT LED intensity atualizada: %u (modo flash)", static_cast<unsigned>(gConfig.ledIntensity));
    return;
  }

  if (strcmp(topic, gMqttCommandCapturePhotoTopic.c_str()) == 0) {
    if (!isCaptureCommandPayload(payload)) {
      LOGW("MQTT capture payload invalido: %s", payload);
      return;
    }

    LOGI("MQTT comando manual de foto recebido");
    sendManualPhoto();
    return;
  }

  if (strcmp(topic, gMqttCommandStreamTopic.c_str()) == 0) {
    bool enabled = false;
    if (!parseBoolPayload(payload, enabled)) {
      LOGW("MQTT stream payload invalido: %s", payload);
      return;
    }

    stream_service::setEnabled(enabled);
    return;
  }

  LOGD("MQTT topic ignorado: %s", topic);
}

void printBootInfo() {
  LOGI("Firmware iniciado");
  LOGI("Device ID MQTT: %s", gMqttDeviceId.c_str());
  LOGI("MQTT root: devices/%s", gMqttDeviceId.c_str());
  LOGI("Heap livre: %lu", static_cast<unsigned long>(ESP.getFreeHeap()));
  serial_cli::printHelp();
}

void setupConfig() {
  app::Status st = config_store::load(gConfig);
  if (st != app::Status::kOk) {
    LOGW("Falha ao carregar config NVS, usando defaults");
  }
}

void setupCamera() {
  if (!tryStartCamera()) {
    LOGE("Camera indisponivel, tentara novamente no loop");
  }
}

void setupNetwork() {
  app::Status st = wifi_service::connect(gConfig);
  if (st != app::Status::kOk) {
    LOGW("WiFi nao conectado no boot");
  }
}

void processRoll() {
  if (!gRoll.update() || !gRoll.hasEvent()) {
    return;
  }

  if (millis() - gLastEventMs < app::kPostEventCooldownMs) {
    gRoll.consumeEvent();
    return;
  }

  RollMetrics metrics = gRoll.consumeEvent();
  gLastEventMs = millis();

  LOGI("-----");
  LOGI("Pulsos: %lu", static_cast<unsigned long>(metrics.pulses));
  LOGI("Tempo roll: %lu us", static_cast<unsigned long>(metrics.rollTimeUs));
  LOGI("Entropia: %lu", static_cast<unsigned long>(metrics.entropy));
  LOGI("D20: %u", static_cast<unsigned>(metrics.dice));

  sendRollEvent(metrics);
}

void processCameraRetry() {
  if (camera_service::isReady()) {
    return;
  }

  uint32_t now = millis();
  if (now - gLastCameraRetryMs < app::kCameraRetryIntervalMs) {
    return;
  }

  LOGW("Tentando reinicializar camera");
  if (!tryStartCamera()) {
    LOGW("Camera ainda indisponivel");
  }
}

}  // namespace

void setup() {
  Serial.begin(app::kSerialBaud);
  Serial.setDebugOutput(true);
  loggerInit(app::kLogsEnabledByDefault);
  setupMqttTopics();
  serial_cli::setMqttDeviceId(gMqttDeviceId);

  setupConfig();
  gRoll.begin(app::kSw420Pin, app::kRollSettleMs);
  setupCamera();
  camera_service::applyLedState(false, 0);
  setupNetwork();
  stream_service::begin();
  mqtt_service::begin();
  mqtt_service::setLogTopic(gMqttLogsTopic.c_str());
  mqtt_service::setMessageCallback(handleMqttMessage);
  if (!mqtt_service::subscribe(gMqttConfigTopicWildcard.c_str())) {
    LOGW("Falha ao registrar subscribe MQTT config");
  }
  if (!mqtt_service::subscribe(gMqttCommandsTopicWildcard.c_str())) {
    LOGW("Falha ao registrar subscribe MQTT commands");
  }
  loggerSetSink(mqtt_service::publishLog);
  printBootInfo();
}

void loop() {
  serial_cli::handle(gConfig);
  mqtt_service::loop();
  stream_service::loop();
  processCameraRetry();
  processRoll();
  delay(app::kLoopDelayMs);
}
