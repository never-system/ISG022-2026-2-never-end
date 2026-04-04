#include "debug/logger.h"

#include <stdarg.h>
#include <stdio.h>

namespace {

bool g_loggerEnabled = true;
LoggerSink g_sink = nullptr;
bool g_inSinkDispatch = false;

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

}  // namespace

void loggerInit(bool enabledByDefault) { g_loggerEnabled = enabledByDefault; }

void loggerSetEnabled(bool enabled) { g_loggerEnabled = enabled; }

bool loggerIsEnabled() { return g_loggerEnabled; }

void loggerSetSink(LoggerSink sink) { g_sink = sink; }

void logPrint(LogLevel level, const char *fmt, ...) {
  if (!g_loggerEnabled && level != LogLevel::kError) {
    return;
  }

  char message[220];
  va_list args;
  va_start(args, fmt);
  vsnprintf(message, sizeof(message), fmt, args);
  va_end(args);

  Serial.printf("[%s] %s\n", levelTag(level), message);

  if (g_sink != nullptr && !g_inSinkDispatch) {
    g_inSinkDispatch = true;
    g_sink(level, message);
    g_inSinkDispatch = false;
  }
}
