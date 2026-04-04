#include "network/mqtt_service.h"

#include <PubSubClient.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>

#include "system/app_config.h"

namespace {

WiFiClientSecure gTlsClient;
PubSubClient gMqttClient(gTlsClient);
uint32_t gLastConnectAttemptMs = 0;
bool gStarted = false;

const char *levelTag(LogLevel level) {
  switch (level) {
    case LogLevel::kError:
      return "E";
    case LogLevel::kWarn:
      return "W";
    case LogLevel::kInfo:
      return "I";
    case LogLevel::kDebug:
      return "D";
    default:
      return "?";
  }
}

String buildClientId() {
  uint64_t chipId = ESP.getEfuseMac();
  char clientId[40];
  snprintf(clientId, sizeof(clientId), "esp32-logs-%08lx", static_cast<unsigned long>(chipId & 0xFFFFFFFFULL));
  return String(clientId);
}

void ensureConnected() {
  if (!gStarted || gMqttClient.connected()) {
    return;
  }

  if (WiFi.status() != WL_CONNECTED) {
    return;
  }

  uint32_t now = millis();
  if (now - gLastConnectAttemptMs < app::kMqttReconnectIntervalMs) {
    return;
  }
  gLastConnectAttemptMs = now;

  String clientId = buildClientId();
  gMqttClient.connect(clientId.c_str(), app::kMqttUsername, app::kMqttPassword);
}

}  // namespace

namespace mqtt_service {

void begin() {
  gTlsClient.setInsecure();
  gTlsClient.setTimeout(app::kMqttSocketTimeoutSec);

  gMqttClient.setServer(app::kMqttHost, app::kMqttPort);
  gMqttClient.setBufferSize(512);
  gStarted = true;

  ensureConnected();
}

void loop() {
  if (!gStarted) {
    return;
  }

  ensureConnected();
  if (gMqttClient.connected()) {
    gMqttClient.loop();
  }
}

bool isConnected() { return gMqttClient.connected(); }

void publishLog(LogLevel level, const char *message) {
  if (!gStarted || message == nullptr || message[0] == '\0') {
    return;
  }

  ensureConnected();
  if (!gMqttClient.connected()) {
    return;
  }

  char payload[300];
  snprintf(payload, sizeof(payload), "[%s] %s", levelTag(level), message);
  gMqttClient.publish(app::kMqttLogsTopic, payload);
}

}  // namespace mqtt_service
