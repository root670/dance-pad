#include "Sensor.h"

// Time in milliseconds of inactivity to cause a recalibration.
uint32_t s_calibrationPeriodMS = 10000; // 10 seconds

void Sensor::calibrate()
{
    m_nTriggerThreshold = m_nPressure + m_nTriggerOffset;
    m_nReleaseThreshold = m_nPressure + m_nReleaseOffset;
}

void Sensor::readSensor()
{
    m_nPressure = analogRead(m_nPin);
}

void Sensor::update()
{
    uint32_t nCurrentTimeMS = millis();
    readSensor();

    if (!m_bPressed)
    {
        if (m_nPressure >= m_nTriggerThreshold)
        {
            m_bPressed = true;
            m_nLastChangeTimeMS = nCurrentTimeMS;
        }
        else if (nCurrentTimeMS - m_nLastChangeTimeMS > s_calibrationPeriodMS)
        {
            calibrate();
            m_nLastChangeTimeMS = nCurrentTimeMS;
        }
    }
    else if (m_nPressure <= m_nReleaseThreshold)
    {
        m_bPressed = false;
        m_nLastChangeTimeMS = nCurrentTimeMS;
    }
}