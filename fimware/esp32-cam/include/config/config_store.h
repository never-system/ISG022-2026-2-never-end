#ifndef CONFIG_STORE_H
#define CONFIG_STORE_H

#include <Arduino.h>

#include "system/status.h"

struct AppConfig {
  String ssid;
  String password;
  String webhookUrl;
};

namespace config_store {

app::Status load(AppConfig &out);
app::Status save(const AppConfig &cfg);
app::Status setWebhook(const String &url, AppConfig &cfg);
app::Status setWifi(const String &ssid, const String &password, AppConfig &cfg);
app::Status resetToDefaults(AppConfig &cfg);

bool isWebhookValid(const String &url);
bool isWifiConfigured(const AppConfig &cfg);

}  // namespace config_store

#endif
