#ifndef WEBHOOK_CLIENT_H
#define WEBHOOK_CLIENT_H

#include "esp_camera.h"

#include "motor/roll_detector.h"
#include "system/status.h"

namespace webhook_client {

app::Status sendPhotoMultipart(const String &url, camera_fb_t *fb, const RollMetrics &m);
app::Status sendJsonFallback(const String &url, const RollMetrics &m);

}  // namespace webhook_client

#endif
