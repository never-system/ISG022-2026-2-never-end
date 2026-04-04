#ifndef APP_CONFIG_H
#define APP_CONFIG_H

#include <Arduino.h>
#include "esp_camera.h"

namespace app {

static constexpr uint8_t kSw420Pin = 13;
static constexpr uint32_t kRollSettleMs = 1000;
static constexpr uint32_t kLoopDelayMs = 2;
static constexpr uint32_t kPostEventCooldownMs = 300;

static constexpr uint32_t kWifiConnectTimeoutMs = 15000;
static constexpr uint32_t kWifiRetryDelayMs = 500;
static constexpr uint32_t kWifiReconnectBaseMs = 2000;
static constexpr uint32_t kWifiReconnectMaxMs = 15000;

static constexpr uint32_t kHttpResponseTimeoutMs = 10000;
static constexpr size_t kUploadChunkSize = 1024;

static constexpr const char *kMqttHost = "mqtt.astrum.app.br";
static constexpr uint16_t kMqttPort = 8883;
static constexpr const char *kMqttUsername = "esp32";
static constexpr const char *kMqttPassword = "esp32";
static constexpr const char *kMqttLogsTopic = "devices/esp32/logs/dev";
static constexpr uint32_t kMqttReconnectIntervalMs = 5000;
static constexpr uint32_t kMqttSocketTimeoutSec = 5;

static constexpr uint32_t kCameraXclkHz = 20000000;
static constexpr int kCameraFrameSize = FRAMESIZE_UXGA;
static constexpr int kCameraJpegQualityNoPsram = 6;
static constexpr int kCameraJpegQualityPsram = 6;
static constexpr uint8_t kCameraFbCountNoPsram = 1;
static constexpr uint8_t kCameraFbCountPsram = 2;

static constexpr uint32_t kSerialBaud = 115200;

static constexpr bool kLogsEnabledByDefault = true;

static constexpr const char *kPrefsNamespace = "cfg";
static constexpr const char *kKeyWebhook = "webhook";
static constexpr const char *kKeySsid = "ssid";
static constexpr const char *kKeyPassword = "pwd";

static constexpr const char *kDefaultSsid = "";
static constexpr const char *kDefaultPassword = "";
static constexpr const char *kDefaultWebhookUrl =
    "https://fronthook.astrum.app.br/hooks/01KKDEQPT7TQWB2ENM5S0SF5BC";

static constexpr const char *kSourceTag = "esp32-cam-sw420";

}  // namespace app

#endif
