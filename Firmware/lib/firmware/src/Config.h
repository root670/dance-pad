//
// Configurable values that can be changed without reflashing the firmware.
//
#pragma once
#include <Arduino.h>
#include <EEPROM.h>
#include <map>
#include <vector>

class Configuration
{
public:

    // Get singleton instance
    static Configuration* getInstance();

    // Strings
    const String& getString(const String &strKey, const String &strDefault);
    void setString(const String &strKey, const String &strValue);

    // 16-bit unsigned integers
    const uint16_t& getUInt16(const String &strKey, const uint16_t &nDefault);
    void setUInt16(const String &strKey, uint16_t nValue);

    // 32-bit unsigned integers
    const uint32_t& getUInt32(const String &strKey, const uint32_t &nDefault);
    void setUInt32(const String &strKey, uint32_t nValue);

    // Read configuration from EEPROM
    void read();

    // Write configuration to EEPROM if data is dirty
    void write();

    // Reset all configuration items in memory
    void reset();

    // Get string representation of configuration items in the form
    // `KEY1=VALUE1,KEY2=VALUE2,...,KEYN=VALUEN`.
    String toString()const;

    typedef void (*pFnConfigCallback)();

    // Set a function to be called when the configuration is updated by the set*() methods
    void registerCallback(pFnConfigCallback cb);

private:

    static Configuration *m_inst;

    Configuration()
        : m_bDirty(false)
    {};

    // Write a null-terminated string to the EEPROM at nOffset.
    // Returns offset immediately after the null-terminator.
    int put(int nOffset, const String &str)const;

    // Write a POD value to the EEPROM at nOffset.
    // Returns the offset immediately after the value.
    template<typename TYPE>
    int put(int nOffset, TYPE nValue)const
    {
        EEPROM.put(nOffset, nValue);
        return nOffset + sizeof(nValue);
    }

    // Get a null-terminated string from the EEPROM at nOffset.
    // Returns offset immediately after the null-terminator.
    int get(int nOffset, String &str)const;

    // Get a POD value from the EEPROM at nOffset.
    // Returns offset immediately after the value.
    template<typename TYPE>
    int get(int nOffset, TYPE &nValue)const
    {
        EEPROM.get(nOffset, nValue);
        Serial.print("get: ");
        Serial.print(static_cast<unsigned int>(nValue), 16);
        Serial.println();
        return nOffset + sizeof(nValue);
    }

    // Call registered callbacks to indicate the configuration has been updated
    void notifyCallbacks()const;

    bool                            m_bDirty;
    std::map<String, String>        m_mapStr;
    std::map<String, uint16_t>      m_mapUInt16;
    std::map<String, uint32_t>      m_mapUInt32;
    std::vector<pFnConfigCallback>  m_vCallbacks;
};
