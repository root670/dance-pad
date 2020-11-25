//
// Code to control lights.
//
#pragma once
#include <cstdint>
#include <FastLED.h>

// Wrapper around FastLED for each a single network of RGB LEDs oriented
// in a grid pattern.
class RGBLights
{
public:
    RGBLights(int nDataPin = -1, int nNumLEDs = 25, int nMaxBrightness = 64) {};
    void setSingle(int row, int col, uint8_t r, uint8_t g, uint8_t b);

private:
    //FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
};
