#ifndef ROLL_DETECTOR_H
#define ROLL_DETECTOR_H

#include <Arduino.h>

struct RollMetrics {
  uint32_t pulses;
  uint32_t rollTimeUs;
  uint32_t entropy;
  uint8_t dice;
};

class RollDetector {
 public:
  void begin(uint8_t pin, uint32_t settleMs);
  bool update();
  bool hasEvent() const;
  RollMetrics consumeEvent();

 private:
  uint8_t pin_ = 0;
  uint32_t settleMs_ = 1000;
  int lastState_ = LOW;
  bool rolling_ = false;
  uint32_t lastVibrationMs_ = 0;
  uint32_t startRollUs_ = 0;
  uint32_t pulses_ = 0;
  uint32_t entropy_ = 0;

  RollMetrics event_{};
  bool eventReady_ = false;

  void onPulse(uint32_t nowUs, uint32_t nowMs);
  void finalizeRoll(uint32_t nowUs);
  uint8_t toDice(uint32_t entropy) const;
};

#endif
