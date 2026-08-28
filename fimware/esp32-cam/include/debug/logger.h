#ifndef LOGGER_H
#define LOGGER_H

#include <Arduino.h>

enum class LogLevel : uint8_t {
  kError = 0,
  kWarn,
  kInfo,
  kDebug
};

using LoggerSink = void (*)(LogLevel level, const char *message);

void loggerInit(bool enabledByDefault);
void loggerSetEnabled(bool enabled);
bool loggerIsEnabled();
void loggerSetSink(LoggerSink sink);

void logPrint(LogLevel level, const char *fmt, ...);

#define LOGE(fmt, ...) logPrint(LogLevel::kError, fmt, ##__VA_ARGS__)
#define LOGW(fmt, ...) logPrint(LogLevel::kWarn, fmt, ##__VA_ARGS__)
#define LOGI(fmt, ...) logPrint(LogLevel::kInfo, fmt, ##__VA_ARGS__)
#define LOGD(fmt, ...) logPrint(LogLevel::kDebug, fmt, ##__VA_ARGS__)

#endif
