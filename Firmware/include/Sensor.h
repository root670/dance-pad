#pragma once

#include <Arduino.h>

#include "Config.h"

class Sensor
{
public:
    Sensor(uint8_t nPin)
    {
        String strIdentifier("sensor" + nPin);

        m_nPin              = nPin;
        m_nPressure         = 0;
        m_nTriggerOffset    = g_config.getUInt16(strIdentifier + "trigger", 50);
        m_nReleaseOffset    = g_config.getUInt16(strIdentifier + "release", 25);
        m_nTriggerThreshold = 0;
        m_nReleaseThreshold = 0;
        m_nLastChangeTimeMS = 0;
        m_bPressed          = false;
    }

    // Set the thresholds based on the most recent reading
    void calibrate();

    // Read value from pin
    void readSensor();

    // Update state of the sensor
    void update();

    uint8_t m_nPin;
    uint16_t m_nPressure;
    uint16_t m_nTriggerOffset; // Amount above the baseline to trigger a hit.
    uint16_t m_nReleaseOffset; // Amount above the baseline to trigger a release.
    uint16_t m_nTriggerThreshold;
    uint16_t m_nReleaseThreshold;
    uint64_t m_nLastChangeTimeMS;
    bool m_bPressed;
};
