#ifndef STREAM_SERVICE_H
#define STREAM_SERVICE_H

#include <Arduino.h>

namespace stream_service {

void begin();
void loop();
void setEnabled(bool enabled);
bool isEnabled();
String streamUrl();

}  // namespace stream_service

#endif
