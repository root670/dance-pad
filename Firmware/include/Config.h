//
// Configurable values that can be changed without reflashing the firmware.
//
#pragma once
#include <Arduino.h>

class Configuration
{
public:
    Configuration() {};

    typedef enum configVersion
    {
        enumVersion1 = 1,
        enumVersionCurrent = enumVersion1
    } configVersion_t;

    /*
    Representation in EEPROM:
    setinel value
    version
    crc of values 1 through n
    value 1
    value 2
    ...
    value n
    */

    bool readFromEEPROM();
    bool writeToEEPROM();
};

Configuration g_config;
