#include <Arduino.h>

#include "camera/camera_service.h"
#include "config/config_store.h"
#include "config/serial_cli.h"
#include "debug/logger.h"
#include "motor/roll_detector.h"
#include "network/mqtt_service.h"
#include "network/webhook_client.h"
#include "network/wifi_service.h"
#include "system/app_config.h"

namespace {

AppConfig gConfig;
RollDetector gRoll;
uint32_t gLastEventMs = 0;

void printBootInfo() {
  LOGI("Firmware iniciado");
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
  app::Status st = camera_service::init();
  if (st != app::Status::kOk) {
    LOGE("Camera indisponivel, tentara novamente no loop");
  }
}

void setupNetwork() {
  app::Status st = wifi_service::connect(gConfig);
  if (st != app::Status::kOk) {
    LOGW("WiFi nao conectado no boot");
  }
}

void sendRollEvent(const RollMetrics &metrics) {
  app::Status wifiSt = wifi_service::ensureConnected(gConfig);
  if (wifiSt != app::Status::kOk) {
    LOGE("Sem WiFi para enviar evento");
    return;
  }

  camera_fb_t *fb = camera_service::capture();
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

}  // namespace

void setup() {
  Serial.begin(app::kSerialBaud);
  Serial.setDebugOutput(true);
  loggerInit(app::kLogsEnabledByDefault);

  setupConfig();
  gRoll.begin(app::kSw420Pin, app::kRollSettleMs);
  setupCamera();
  setupNetwork();
  mqtt_service::begin();
  loggerSetSink(mqtt_service::publishLog);
  printBootInfo();
}

void loop() {
  serial_cli::handle(gConfig);
  mqtt_service::loop();
  processRoll();
  delay(app::kLoopDelayMs);
}
