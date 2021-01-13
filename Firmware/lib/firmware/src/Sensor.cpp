#include "Sensor.h"

static const uint16_t kDefaultTriggerOffset = 50;
static const uint16_t kDefaultReleaseOffset = 30;

// Time in milliseconds of inactivity to cause a recalibration.
static uint32_t s_calibrationPeriodMS = 10000; // 10 seconds

Sensor::Sensor(uint8_t nPin)
{
    String strIdentifier("sensor");
    strIdentifier.append(nPin);

    m_nPin              = nPin;
    m_nPressure         = 0;
    m_strTriggerOffsetSetting = strIdentifier + "trigger";
    m_strReleaseOffsetSetting = strIdentifier + "release";
    m_nTriggerOffset    = Configuration::getInstance()->getUInt16(m_strTriggerOffsetSetting, kDefaultTriggerOffset);
    m_nReleaseOffset    = Configuration::getInstance()->getUInt16(m_strReleaseOffsetSetting, kDefaultReleaseOffset);
    m_nTriggerThreshold = 0;
    m_nReleaseThreshold = 0;
    m_nLastChangeTimeMS = 0;
    m_bPressed          = false;
}

void Sensor::calibrate()
{
    m_nTriggerThreshold = m_nPressure + Configuration::getInstance()->getUInt16(m_strTriggerOffsetSetting, kDefaultTriggerOffset);
    m_nReleaseThreshold = m_nPressure + Configuration::getInstance()->getUInt16(m_strReleaseOffsetSetting, kDefaultReleaseOffset);
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

bool Sensor::isPressed()const
{
    return m_bPressed;
}

uint16_t Sensor::getPressure()const
{
    return m_nPressure;
}

uint16_t Sensor::getTriggerThreshold()const
{
    return m_nTriggerThreshold;
}

uint16_t Sensor::getReleaseThreshold()const
{
    return m_nReleaseThreshold;
}

uint8_t Sensor::getPin()const
{
    return m_nPin;
}