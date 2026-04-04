#include "config/serial_cli.h"

#include "config/config_store.h"
#include "debug/logger.h"

namespace {

const String kCmdShowCfg = "SHOWCFG";
const String kCmdResetCfg = "RESETCFG";
const String kCmdLogOn = "LOG ON";
const String kCmdLogOff = "LOG OFF";
const String kCmdSetWebhook = "SETWEBHOOK ";
const String kCmdSetSsid = "SETSSID ";
const String kCmdSetPass = "SETPASS ";
const size_t kMaxCmdLen = 256;

void printConfig(const AppConfig &cfg) {
  Serial.printf("SSID: %s\n", cfg.ssid.c_str());
  Serial.printf("PASS: %s\n", cfg.password.length() ? "***" : "<vazio>");
  Serial.printf("WEBHOOK: %s\n", cfg.webhookUrl.c_str());
  Serial.printf("LOGS: %s\n", loggerIsEnabled() ? "ON" : "OFF");
}

bool startsWithIgnoreCase(const String &value, const String &prefix) {
  String lhs = value;
  String rhs = prefix;
  lhs.toUpperCase();
  rhs.toUpperCase();
  return lhs.startsWith(rhs);
}

}  // namespace

namespace serial_cli {

void printHelp() {
  Serial.println("Comandos:");
  Serial.println("SHOWCFG");
  Serial.println("SETSSID <ssid>");
  Serial.println("SETPASS <senha>");
  Serial.println("SETWEBHOOK <url>");
  Serial.println("RESETCFG");
  Serial.println("LOG ON | LOG OFF");
}

void handle(AppConfig &cfg) {
  if (!Serial.available()) {
    return;
  }

  String cmd = Serial.readStringUntil('\n');
  cmd.trim();

  if (cmd.length() == 0 || cmd.length() > kMaxCmdLen) {
    printHelp();
    return;
  }

  if (cmd.equalsIgnoreCase(kCmdShowCfg)) {
    printConfig(cfg);
    return;
  }

  if (cmd.equalsIgnoreCase(kCmdResetCfg)) {
    app::Status st = config_store::resetToDefaults(cfg);
    Serial.println(st == app::Status::kOk ? "RESETCFG OK" : "RESETCFG FALHOU");
    return;
  }

  if (cmd.equalsIgnoreCase(kCmdLogOn)) {
    loggerSetEnabled(true);
    Serial.println("LOG ON");
    return;
  }

  if (cmd.equalsIgnoreCase(kCmdLogOff)) {
    loggerSetEnabled(false);
    Serial.println("LOG OFF");
    return;
  }

  if (startsWithIgnoreCase(cmd, kCmdSetWebhook)) {
    String value = cmd.substring(kCmdSetWebhook.length());
    value.trim();
    app::Status st = config_store::setWebhook(value, cfg);
    Serial.println(st == app::Status::kOk ? "SETWEBHOOK OK" : "SETWEBHOOK FALHOU");
    return;
  }

  if (startsWithIgnoreCase(cmd, kCmdSetSsid)) {
    String value = cmd.substring(kCmdSetSsid.length());
    value.trim();
    app::Status st = config_store::setWifi(value, cfg.password, cfg);
    Serial.println(st == app::Status::kOk ? "SETSSID OK" : "SETSSID FALHOU");
    return;
  }

  if (startsWithIgnoreCase(cmd, kCmdSetPass)) {
    String value = cmd.substring(kCmdSetPass.length());
    value.trim();
    app::Status st = config_store::setWifi(cfg.ssid, value, cfg);
    Serial.println(st == app::Status::kOk ? "SETPASS OK" : "SETPASS FALHOU");
    return;
  }

  printHelp();
}

}  // namespace serial_cli
