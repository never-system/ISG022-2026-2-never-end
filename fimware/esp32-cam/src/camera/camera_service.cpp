#include "camera/camera_service.h"

#include <Arduino.h>

#include "board_config.h"
#include "camera_pins.h"
#include "debug/logger.h"
#include "system/app_config.h"

namespace {

constexpr uint8_t kLedPwmChannel = 7;
constexpr uint8_t kLedPwmResolutionBits = 8;
constexpr uint32_t kLedPwmFrequencyHz = 5000;

bool gLedPwmReady = false;
bool gCameraReady = false;

void ensureLedReady() {
  if (gLedPwmReady) {
    return;
  }

  ledcSetup(kLedPwmChannel, kLedPwmFrequencyHz, kLedPwmResolutionBits);
  ledcAttachPin(LED_GPIO_NUM, kLedPwmChannel);
  gLedPwmReady = true;
}

camera_config_t buildConfig() {
  const bool hasPsram = psramFound();

  camera_config_t cfg{};
  cfg.ledc_channel = LEDC_CHANNEL_0;
  cfg.ledc_timer = LEDC_TIMER_0;
  cfg.pin_d0 = Y2_GPIO_NUM;
  cfg.pin_d1 = Y3_GPIO_NUM;
  cfg.pin_d2 = Y4_GPIO_NUM;
  cfg.pin_d3 = Y5_GPIO_NUM;
  cfg.pin_d4 = Y6_GPIO_NUM;
  cfg.pin_d5 = Y7_GPIO_NUM;
  cfg.pin_d6 = Y8_GPIO_NUM;
  cfg.pin_d7 = Y9_GPIO_NUM;
  cfg.pin_xclk = XCLK_GPIO_NUM;
  cfg.pin_pclk = PCLK_GPIO_NUM;
  cfg.pin_vsync = VSYNC_GPIO_NUM;
  cfg.pin_href = HREF_GPIO_NUM;
  cfg.pin_sccb_sda = SIOD_GPIO_NUM;
  cfg.pin_sccb_scl = SIOC_GPIO_NUM;
  cfg.pin_pwdn = PWDN_GPIO_NUM;
  cfg.pin_reset = RESET_GPIO_NUM;
  cfg.xclk_freq_hz = app::kCameraXclkHz;
  cfg.pixel_format = PIXFORMAT_JPEG;
  cfg.frame_size = static_cast<framesize_t>(
      hasPsram ? app::kCameraFrameSizePsram : app::kCameraFrameSizeNoPsram);
  cfg.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
  cfg.fb_location = hasPsram ? CAMERA_FB_IN_PSRAM : CAMERA_FB_IN_DRAM;
  cfg.jpeg_quality = app::kCameraJpegQualityNoPsram;
  cfg.fb_count = app::kCameraFbCountNoPsram;

  if (hasPsram) {
    cfg.jpeg_quality = app::kCameraJpegQualityPsram;
    cfg.fb_count = app::kCameraFbCountPsram;
    cfg.grab_mode = CAMERA_GRAB_LATEST;
  }
  return cfg;
}

void tuneSensor() {
  sensor_t *s = esp_camera_sensor_get();
  if (s == nullptr) {
    return;
  }

  if (s->id.PID == OV3660_PID) {
    s->set_vflip(s, 1);
    s->set_brightness(s, 1);
    s->set_saturation(s, -2);
  }
  s->set_framesize(s, static_cast<framesize_t>(
                          psramFound() ? app::kCameraFrameSizePsram
                                       : app::kCameraFrameSizeNoPsram));
}

}  // namespace

namespace camera_service {

app::Status init() {
  if (gCameraReady) {
    return app::Status::kOk;
  }

  camera_config_t cfg = buildConfig();
  esp_err_t err = esp_camera_init(&cfg);
  if (err != ESP_OK) {
    gCameraReady = false;
    LOGE("Camera init falhou: 0x%x", static_cast<unsigned>(err));
    return app::Status::kCameraError;
  }

  tuneSensor();
  gCameraReady = true;
  LOGI("Camera inicializada");
  return app::Status::kOk;
}

bool isReady() { return gCameraReady; }

void applyLedState(bool power, uint8_t intensity) {
  ensureLedReady();
  const uint32_t duty = power ? static_cast<uint32_t>(intensity) : 0;
  ledcWrite(kLedPwmChannel, duty);
}

camera_fb_t *capture() {
  if (!gCameraReady) {
    return nullptr;
  }

  camera_fb_t *fb = esp_camera_fb_get();
  if (fb == nullptr) {
    LOGW("Camera nao retornou frame");
  }
  return fb;
}

void release(camera_fb_t *fb) {
  if (fb != nullptr) {
    esp_camera_fb_return(fb);
  }
}

}  // namespace camera_service
