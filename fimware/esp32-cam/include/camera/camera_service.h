#ifndef CAMERA_SERVICE_H
#define CAMERA_SERVICE_H

#include "esp_camera.h"

#include "system/status.h"

namespace camera_service {

app::Status init();
camera_fb_t *capture();
void release(camera_fb_t *fb);

}  // namespace camera_service

#endif
