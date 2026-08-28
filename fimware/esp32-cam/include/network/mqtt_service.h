#ifndef MQTT_SERVICE_H
#define MQTT_SERVICE_H

#include "debug/logger.h"

namespace mqtt_service {

using MqttMessageCallback = void (*)(const char *topic, const char *payload);

void begin();
void loop();
bool isConnected();
void setMessageCallback(MqttMessageCallback callback);
void setLogTopic(const char *topic);
bool subscribe(const char *topic);
void publishLog(LogLevel level, const char *message);

}  // namespace mqtt_service

#endif
