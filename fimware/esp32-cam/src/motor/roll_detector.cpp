#include "motor/roll_detector.h"

#include <esp_system.h>

void RollDetector::begin(uint8_t pin, uint32_t settleMs) {
  pin_ = pin;
  settleMs_ = settleMs;
  pinMode(pin_, INPUT);
  lastState_ = digitalRead(pin_);
  entropy_ = micros();
}

bool RollDetector::update() {
  int current = digitalRead(pin_);
  uint32_t nowUs = micros();
  uint32_t nowMs = millis();

  if (lastState_ == LOW && current == HIGH) {
    onPulse(nowUs, nowMs);
  }

  if (rolling_ && (nowMs - lastVibrationMs_ > settleMs_)) {
    finalizeRoll(nowUs);
  }

  lastState_ = current;
  return eventReady_;
}

bool RollDetector::hasEvent() const { return eventReady_; }

RollMetrics RollDetector::consumeEvent() {
  eventReady_ = false;
  return event_;
}

void RollDetector::onPulse(uint32_t nowUs, uint32_t nowMs) {
  if (!rolling_) {
    startRollUs_ = nowUs;
  }

  rolling_ = true;
  lastVibrationMs_ = nowMs;
  pulses_++;

  entropy_ ^= nowUs;
  entropy_ ^= (pulses_ * 2654435761UL);
  entropy_ ^= static_cast<uint32_t>(esp_random());
}

void RollDetector::finalizeRoll(uint32_t nowUs) {
  const uint32_t rollTime = nowUs - startRollUs_;
  entropy_ ^= rollTime;
  entropy_ ^= (pulses_ << 16);
  entropy_ ^= micros();

  event_.pulses = pulses_;
  event_.rollTimeUs = rollTime;
  event_.entropy = entropy_;
  event_.dice = toDice(entropy_);
  eventReady_ = true;

  pulses_ = 0;
  entropy_ = micros();
  rolling_ = false;
}

uint8_t RollDetector::toDice(uint32_t entropy) const {
  return static_cast<uint8_t>((entropy % 20U) + 1U);
}
