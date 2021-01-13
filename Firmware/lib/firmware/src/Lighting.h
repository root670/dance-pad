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

namespace Lights
{
    // Initialize lighting dependencies
    void initialize();

    // Set all LEDs in a strip to a color
    void illuminateStrip(lightIdentifier_t id, const CRGB &color);

    // Set lights as enabled or disabled
    void setStatus(lightIdentifier_t id, bool bEnabled);

    // Fade the current LEDs in all strips
    void update();
}
