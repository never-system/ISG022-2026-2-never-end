#include "config/config_store.h"

#include <Preferences.h>

#include "system/app_config.h"

namespace {

Preferences gPrefs;

bool beginPrefs() { return gPrefs.begin(app::kPrefsNamespace, false); }

void fillDefaults(AppConfig &cfg) {
  cfg.ssid = app::kDefaultSsid;
  cfg.password = app::kDefaultPassword;
  cfg.webhookUrl = app::kDefaultWebhookUrl;
  cfg.ledPower = app::kDefaultLedPower;
  cfg.ledIntensity = app::kDefaultLedIntensity;
}

}  // namespace

namespace config_store {

bool isWebhookValid(const String &url) {
  return url.startsWith("http://") || url.startsWith("https://");
}

bool isWifiConfigured(const AppConfig &cfg) { return cfg.ssid.length() > 0; }

app::Status load(AppConfig &out) {
  fillDefaults(out);
  if (!beginPrefs()) {
    return app::Status::kStorageError;
  }

  out.ssid = gPrefs.getString(app::kKeySsid, app::kDefaultSsid);
  out.password = gPrefs.getString(app::kKeyPassword, app::kDefaultPassword);
  out.webhookUrl = gPrefs.getString(app::kKeyWebhook, app::kDefaultWebhookUrl);
  out.ledPower = gPrefs.getBool(app::kKeyLedPower, app::kDefaultLedPower);
  out.ledIntensity = gPrefs.getUChar(app::kKeyLedIntensity, app::kDefaultLedIntensity);
  gPrefs.end();

  if (!isWebhookValid(out.webhookUrl)) {
    out.webhookUrl = app::kDefaultWebhookUrl;
  }
  return app::Status::kOk;
}

app::Status save(const AppConfig &cfg) {
  if (!isWebhookValid(cfg.webhookUrl)) {
    return app::Status::kInvalidArgument;
  }

  if (!beginPrefs()) {
    return app::Status::kStorageError;
  }

  bool ok = gPrefs.putString(app::kKeySsid, cfg.ssid) > 0 || cfg.ssid.length() == 0;
  ok = ok && (gPrefs.putString(app::kKeyPassword, cfg.password) > 0 || cfg.password.length() == 0);
  ok = ok && gPrefs.putString(app::kKeyWebhook, cfg.webhookUrl) > 0;
  ok = ok && gPrefs.putBool(app::kKeyLedPower, cfg.ledPower);
  ok = ok && gPrefs.putUChar(app::kKeyLedIntensity, cfg.ledIntensity) > 0;
  gPrefs.end();

  return ok ? app::Status::kOk : app::Status::kStorageError;
}

app::Status setWebhook(const String &url, AppConfig &cfg) {
  if (!isWebhookValid(url)) {
    return app::Status::kInvalidArgument;
  }
  cfg.webhookUrl = url;
  return save(cfg);
}

app::Status setWifi(const String &ssid, const String &password, AppConfig &cfg) {
  if (ssid.length() == 0) {
    return app::Status::kInvalidArgument;
  }
  cfg.ssid = ssid;
  cfg.password = password;
  return save(cfg);
}

app::Status setLedPower(bool power, AppConfig &cfg) {
  cfg.ledPower = power;
  return save(cfg);
}

app::Status setLedIntensity(uint8_t intensity, AppConfig &cfg) {
  cfg.ledIntensity = intensity;
  return save(cfg);
}

app::Status resetToDefaults(AppConfig &cfg) {
  fillDefaults(cfg);
  return save(cfg);
}

}  // namespace config_store
