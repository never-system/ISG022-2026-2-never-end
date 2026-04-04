#include "network/wifi_service.h"

#include <WiFi.h>

#include "debug/logger.h"
#include "system/app_config.h"

namespace {

app::Status waitForConnection(uint32_t timeoutMs) {
  uint32_t start = millis();
  while (WiFi.status() != WL_CONNECTED) {
    if (millis() - start > timeoutMs) {
      return app::Status::kTimeout;
    }
    delay(app::kWifiRetryDelayMs);
  }
  return app::Status::kOk;
}

}  // namespace

namespace wifi_service {

app::Status connect(const AppConfig &cfg) {
  if (!config_store::isWifiConfigured(cfg)) {
    LOGW("WiFi nao configurado. Use SETSSID/SETPASS no Serial");
    return app::Status::kInvalidArgument;
  }

  WiFi.setSleep(false);
  WiFi.begin(cfg.ssid.c_str(), cfg.password.c_str());
  app::Status st = waitForConnection(app::kWifiConnectTimeoutMs);
  if (st != app::Status::kOk) {
    LOGE("Timeout conectando no WiFi");
    return st;
  }

  LOGI("WiFi ok: %s", WiFi.localIP().toString().c_str());
  return app::Status::kOk;
}

app::Status ensureConnected(const AppConfig &cfg) {
  if (WiFi.status() == WL_CONNECTED) {
    return app::Status::kOk;
  }

  uint32_t delayMs = app::kWifiReconnectBaseMs;
  for (uint8_t attempt = 0; attempt < 3; ++attempt) {
    LOGW("WiFi caiu, tentativa %u", static_cast<unsigned>(attempt + 1));
    app::Status st = connect(cfg);
    if (st == app::Status::kOk) {
      return app::Status::kOk;
    }
    delay(delayMs);
    delayMs = min(delayMs * 2, app::kWifiReconnectMaxMs);
  }
  return app::Status::kNetworkError;
}

}  // namespace wifi_service
