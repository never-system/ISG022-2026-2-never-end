#include "network/mqtt_service.h"

#include <PubSubClient.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>

#include <string.h>

#include "system/app_config.h"

namespace {

WiFiClientSecure gTlsClient;
PubSubClient gMqttClient(gTlsClient);
uint32_t gLastConnectAttemptMs = 0;
bool gStarted = false;
mqtt_service::MqttMessageCallback gMessageCallback = nullptr;

constexpr size_t kMaxSubscriptions = 8;
constexpr size_t kMaxPayloadLen = 400;
constexpr size_t kMaxTopicLen = 96;
const char *gSubscriptions[kMaxSubscriptions]{};
size_t gSubscriptionCount = 0;
char gLogTopic[kMaxTopicLen] = {0};

void onMqttMessage(char *topic, byte *payload, unsigned int length) {
  if (gMessageCallback == nullptr || topic == nullptr) {
    return;
  }

  if (length > kMaxPayloadLen) {
    return;
  }

  char payloadBuffer[kMaxPayloadLen + 1];
  if (length > 0 && payload != nullptr) {
    memcpy(payloadBuffer, payload, length);
  }
  payloadBuffer[length] = '\0';
  gMessageCallback(topic, payloadBuffer);
}

void applySubscriptions() {
  if (!gMqttClient.connected()) {
    return;
  }

  for (size_t i = 0; i < gSubscriptionCount; ++i) {
    if (gSubscriptions[i] != nullptr) {
      gMqttClient.subscribe(gSubscriptions[i]);
    }
  }
}

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
  bool connected = gMqttClient.connect(clientId.c_str(), app::kMqttUsername, app::kMqttPassword);
  if (connected) {
    applySubscriptions();
  }
}

}  // namespace

namespace mqtt_service {

void begin() {
  gTlsClient.setInsecure();
  gTlsClient.setTimeout(app::kMqttSocketTimeoutSec);

  gMqttClient.setServer(app::kMqttHost, app::kMqttPort);
  gMqttClient.setBufferSize(512);
  gMqttClient.setCallback(onMqttMessage);
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

void setMessageCallback(MqttMessageCallback callback) { gMessageCallback = callback; }

void setLogTopic(const char *topic) {
  if (topic == nullptr || topic[0] == '\0') {
    gLogTopic[0] = '\0';
    return;
  }

  snprintf(gLogTopic, sizeof(gLogTopic), "%s", topic);
}

bool subscribe(const char *topic) {
  if (topic == nullptr || topic[0] == '\0') {
    return false;
  }

  for (size_t i = 0; i < gSubscriptionCount; ++i) {
    if (gSubscriptions[i] != nullptr && strcmp(gSubscriptions[i], topic) == 0) {
      return true;
    }
  }

  if (gSubscriptionCount >= kMaxSubscriptions) {
    return false;
  }

  gSubscriptions[gSubscriptionCount++] = topic;
  if (!gMqttClient.connected()) {
    return true;
  }

  return gMqttClient.subscribe(topic);
}

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
  const char *targetTopic = gLogTopic[0] == '\0' ? app::kMqttLogsTopic : gLogTopic;
  gMqttClient.publish(targetTopic, payload);
}

}  // namespace mqtt_service
