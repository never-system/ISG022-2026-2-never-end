#ifndef MQTT_SERVICE_H
#define MQTT_SERVICE_H

#include "debug/logger.h"

namespace mqtt_service {

void begin();
void loop();
bool isConnected();
void publishLog(LogLevel level, const char *message);

}  // namespace mqtt_service

#endif
