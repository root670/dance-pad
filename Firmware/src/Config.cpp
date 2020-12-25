#include "Config.h"
#include <EEPROM.h>

Configuration g_config;
static const uint32_t kSentinelValue = 0x5AFEC0DE;

const String& Configuration::getString(const String &strKey, const String &strDefault)
{
    auto it = m_mapStr.find(strKey);
    if(it == m_mapStr.end())
    {
        m_mapStr[strKey] = strDefault;
    }
    
    return it->second;
}

void Configuration::setString(const String &strKey, const String &strValue)
{
    m_mapStr[strKey] = strValue;
    m_bDirty = true;
}

const uint16_t& Configuration::getUInt16(const String &strKey, const uint16_t &nDefault)
{
    auto it = m_mapUInt16.find(strKey);
    if(it == m_mapUInt16.end())
    {
        m_mapUInt16[strKey] = nDefault;
    }

    return it->second;
}

void Configuration::setUInt16(const String &strKey, uint16_t nValue)
{
    m_mapUInt16[strKey] = nValue;
    m_bDirty = true;
}

const uint32_t& Configuration::getUInt32(const String &strKey, const uint32_t &nDefault)
{
    auto it = m_mapUInt32.find(strKey);
    if(it == m_mapUInt32.end())
    {
        m_mapUInt32[strKey] = nDefault;
    }

    return it->second;
}

void Configuration::setUInt32(const String &strKey, uint32_t nValue)
{
    m_mapUInt32[strKey] = nValue;
    m_bDirty = true;
}

int Configuration::put(int nOffset, const String &str)const
{
    const char *pData = str.c_str();
    while(*pData)
    {
        EEPROM.put(nOffset++, *pData++);
    }
    EEPROM.put(nOffset++, '\0');
    return nOffset;
}

int Configuration::get(int nOffset, String &str)const
{
    char c;
    while(EEPROM.get(nOffset++, c))
    {
        str += c;
    }

    Serial.printf("get[str]: %s\n", str.c_str());
    
    return nOffset;
}

void Configuration::read()
{
    // Get sentinel value
    int nOffset(0);
    uint32_t nSentinel;
    nOffset = get(nOffset, nSentinel);

    if(nSentinel != kSentinelValue)
        return; // Uninitialized or bad data

    // Get strings
    int nNumStrings;
    nOffset = get(nOffset, nNumStrings);
    for(int nString = 0; nString < nNumStrings; nString++)
    {
        String strKey, strValue;
        nOffset = get(nOffset, strKey);
        nOffset = get(nOffset, strValue);
        m_mapStr[strKey] = strValue;
    }

    // Get 32-bit unsigned integers
    int nNumUInt32;
    nOffset = get(nOffset, nNumUInt32);
    for(int nElement = 0; nElement < nNumUInt32; nElement++)
    {
        String strKey;
        uint32_t nValue;
        nOffset = get(nOffset, strKey);
        nOffset = get(nOffset, nValue);
        m_mapUInt32[strKey] = nValue;
    }

    m_bDirty = false;
}

void Configuration::write()
{
    if(m_bDirty)
    {
        // Write sentinel value
        int nOffset(0);
        nOffset = put(nOffset, kSentinelValue);

        // Write strings
        nOffset = put(nOffset, m_mapStr.size());
        for(auto const &element : m_mapStr)
        {
            nOffset = put(nOffset, element.first);
            nOffset = put(nOffset, element.second);
        }

        // Write 32-bit unsigned integers
        nOffset = put(nOffset, m_mapUInt32.size());
        for(auto const &element : m_mapUInt32)
        {
            nOffset = put(nOffset, element.first);
            nOffset = put(nOffset, element.second);
        }
    }
}

String Configuration::toString()const
{
    String strResponse;

    for(auto const &element : m_mapStr)
    {
        strResponse.append(element.first + "=" + element.second + ",");
    }
    for(auto const &element : m_mapUInt32)
    {
        strResponse.append(element.first + "=" + element.second + ",");
    }
    for(auto const &element : m_mapUInt32)
    {
        strResponse.append(element.first + "=" + element.second + ",");
    }

    // Remove trailing comma
    strResponse.setCharAt(strResponse.length() - 1, '\0');

    return strResponse;
}