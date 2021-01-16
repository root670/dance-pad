//
// Code to control lights.
//
#pragma once
#include <cstdint>
#include <FastLED.h>

typedef enum lightIdentifier
{
    enumLightsUpArrow,
    enumLightsDownArrow,
    enumLightsLeftArrow,
    enumLightsRightArrow,
} lightIdentifier_t;

class Lights
{
public:

    // Get singleton instance
    static Lights* getInstance();

    // Set all LEDs in a strip to a color
    void illuminateStrip(lightIdentifier_t id, const CRGB &color);

    // Set lights as enabled or disabled
    void setStatus(lightIdentifier_t id, bool bEnabled);

    // Get color values from config
    void updateColors();

    // Illuminate and fade the current LEDs in all strips
    void update();

    struct Color : CRGB
    {
        using CRGB::CRGB;

        inline uint32_t toUInt32()const
        {
            return r | (g << 8) | (b << 16);
        }

        inline void fromUInt32(uint32_t nColor)
        {
            setRGB(nColor, nColor >> 8, nColor >> 16);
        }
    };

private:
    static Lights *m_pInst;

    Lights();

    Color s_colorUp;
    Color s_colorDown;
    Color s_colorLeft;
    Color s_colorRight;

    bool m_bUp, m_bDown, m_bLeft, m_bRight;
};
