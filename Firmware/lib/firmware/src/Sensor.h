#pragma once

#include <Arduino.h>

#include "Config.h"

class Sensor {
public:
  Sensor(uint8_t nPin);

  // Set the thresholds based on the most recent reading
  void calibrate();

  // Read value from pin
  void readSensor();

  // Update state of the sensor
  void update();

  bool isPressed() const;
  uint16_t getPressure() const;
  uint16_t getTriggerThreshold() const;
  uint16_t getReleaseThreshold() const;
  uint8_t getPin() const;

private:
  uint8_t m_nPin;
  uint16_t m_nPressure;
  uint16_t m_nTriggerOffset; // Amount above the baseline to trigger a hit.
  uint16_t m_nReleaseOffset; // Amount above the baseline to trigger a release.
  String m_strTriggerOffsetSetting; // Config name for trigger offset
  String m_strReleaseOffsetSetting; // Config name for trigger offset setting
  uint16_t m_nTriggerThreshold; // Absolute value to trigger a hit.
  uint16_t m_nReleaseThreshold; // Absolute value to trigger a release.
  uint64_t m_nLastChangeTimeMS;
  bool m_bPressed;
};
