#include <FastLED.h>
#include <OctoWS2811.h>

#include "Lighting.h"
#include "Config.h"

#define NUM_LEDS_PER_STRIP  25
#define NUM_STRIPS          4
#define NUM_LEDS            NUM_LEDS_PER_STRIP * NUM_STRIPS
#define BRIGHTNESS          200
#define COLOR_ORDER         GRB

static CRGB s_ledsRaw[NUM_LEDS];
static CRGB s_ledsCorrected[NUM_LEDS];

#define UPDATES_PER_SECOND  100

// Any group of digital pins may be used by OctoWS2811 on the Teensy 4.1
#define PIN_UP_LED      2
#define PIN_DOWN_LED    3
#define PIN_LEFT_LED    4
#define PIN_RIGHT_LED   5

const byte pinList[NUM_STRIPS] = {
    PIN_UP_LED,
    PIN_DOWN_LED,
    PIN_LEFT_LED,
    PIN_RIGHT_LED
};

// These buffers need to be large enough for all the pixels.
// The total number of pixels is "ledsPerStrip * numPins".
// Each pixel needs 3 bytes, so multiply by 3.  An "int" is
// 4 bytes, so divide by 4.  The array is created using "int"
// so the compiler will align it to 32 bit memory.

static DMAMEM int displayMemory[NUM_LEDS * 3 / 4];
static int drawingMemory[NUM_LEDS * 3 / 4];

template<EOrder RGB_ORDER>
class MyController : public CPixelLEDController<RGB_ORDER>
{
public:
    MyController()
        : m_pOcto(NULL)
    {
        Serial.println("Octo2811B Controller for Teensy 4.1");
    }

    virtual void init() { /* do nothing yet */ }

    virtual void showPixels(PixelController<RGB_ORDER> & pixels)
    {
        _init(pixels.size());

        uint8_t *p = m_pDrawbuffer;

        while(pixels.has(1))
        {
            *p++ = pixels.loadAndScale0();
            *p++ = pixels.loadAndScale1();
            *p++ = pixels.loadAndScale2();
            pixels.stepDithering();
            pixels.advanceData();
        }

        m_pOcto->show();
    }

private:

    void _init(int nLeds)
    {
        if(m_pOcto)
            return; // Already initialized

        m_pDrawbuffer = (uint8_t *)drawingMemory;
        m_pFramebuffer = (uint8_t *)displayMemory;

        int config = WS2811_RGB | WS2811_800kHz;
        // nLeds is the total number of LEDs, but we want the number used by
        // each strip.
        m_pOcto = new OctoWS2811(NUM_LEDS_PER_STRIP, m_pFramebuffer, m_pDrawbuffer, config, NUM_STRIPS, pinList);
        m_pOcto->begin();
    }

    OctoWS2811 *m_pOcto;
    uint8_t *m_pDrawbuffer, *m_pFramebuffer;
};

static MyController<GRB> s_controller;

static const String s_strColorUp("color_up");
static const String s_strColorDown("color_down");
static const String s_strColorLeft("color_left");
static const String s_strColorRight("color_right");
static const Lights::Color s_colorBlue(0x18, 0, 0xff);
static const Lights::Color s_colorMag(0xeb, 0, 0x9b);

static const uint8_t s_gamma8[] = {
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1,
    1,  1,  1,  1,  1,  1,  1,  1,  1,  2,  2,  2,  2,  2,  2,  2,
    2,  3,  3,  3,  3,  3,  3,  3,  4,  4,  4,  4,  4,  5,  5,  5,
    5,  6,  6,  6,  6,  7,  7,  7,  7,  8,  8,  8,  9,  9,  9, 10,
   10, 10, 11, 11, 11, 12, 12, 13, 13, 13, 14, 14, 15, 15, 16, 16,
   17, 17, 18, 18, 19, 19, 20, 20, 21, 21, 22, 22, 23, 24, 24, 25,
   25, 26, 27, 27, 28, 29, 29, 30, 31, 32, 32, 33, 34, 35, 35, 36,
   37, 38, 39, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 50,
   51, 52, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 66, 67, 68,
   69, 70, 72, 73, 74, 75, 77, 78, 79, 81, 82, 83, 85, 86, 87, 89,
   90, 92, 93, 95, 96, 98, 99,101,102,104,105,107,109,110,112,114,
  115,117,119,120,122,124,126,127,129,131,133,135,137,138,140,142,
  144,146,148,150,152,154,156,158,160,162,164,167,169,171,173,175,
  177,180,182,184,186,189,191,193,196,198,200,203,205,208,210,213,
  215,218,220,223,225,228,231,233,236,239,241,244,247,249,252,255
};

Lights* Lights::m_pInst = NULL;

Lights* Lights::getInstance()
{
    if(!m_pInst)
        m_pInst = new Lights();

    return m_pInst;
}

static void OnConfigUpdated()
{
    Lights::getInstance()->updateColors();
}

Lights::Lights()
    : m_bUp(false), m_bDown(false), m_bLeft(false), m_bRight(false)
{
    FastLED.addLeds(&s_controller, s_ledsCorrected, NUM_LEDS).setCorrection(TypicalLEDStrip);
    FastLED.setBrightness(BRIGHTNESS);
    FastLED.setMaxRefreshRate(0); // We will constrain this ourselves

    updateColors();

    Configuration::getInstance()->registerCallback(OnConfigUpdated);
}

void Lights::updateColors()
{
    s_colorUp.fromUInt32(
        Configuration::getInstance()->getUInt32(s_strColorUp, s_colorMag.toUInt32())
    );
    s_colorDown.fromUInt32(
        Configuration::getInstance()->getUInt32(s_strColorDown, s_colorMag.toUInt32())
    );
    s_colorLeft.fromUInt32(
        Configuration::getInstance()->getUInt32(s_strColorLeft, s_colorBlue.toUInt32())
    );
    s_colorRight.fromUInt32(
        Configuration::getInstance()->getUInt32(s_strColorRight, s_colorBlue.toUInt32())
    );
}

void Lights::illuminateStrip(lightIdentifier_t id, const CRGB &color)
{
    for(int nLED = 0; nLED < NUM_LEDS_PER_STRIP; nLED++)
    {
        s_ledsRaw[(int)id*NUM_LEDS_PER_STRIP + nLED] = color;
    }
}

void Lights::setStatus(lightIdentifier_t id, bool bEnabled)
{
    if(id == enumLightsUpArrow)
        m_bUp = bEnabled;
    else if(id == enumLightsDownArrow)
        m_bDown = bEnabled;
    else if(id == enumLightsLeftArrow)
        m_bLeft = bEnabled;
    else if(id == enumLightsRightArrow)
        m_bRight = bEnabled;
}

void Lights::colorCorrect()const
{
    for(int nLED = 0; nLED < NUM_LEDS; nLED++)
    {
        const CRGB &src = s_ledsRaw[nLED];
        CRGB &dst       = s_ledsCorrected[nLED];
        dst.r = s_gamma8[src.r];
        dst.g = s_gamma8[src.g];
        dst.b = s_gamma8[src.b];
    }
}

void Lights::update()
{
    fadeToBlackBy(s_ledsRaw, NUM_LEDS, 20);

    if(m_bUp)
        illuminateStrip(enumLightsUpArrow, s_colorUp);
    if(m_bDown)
        illuminateStrip(enumLightsDownArrow, s_colorDown);
    if(m_bLeft)
        illuminateStrip(enumLightsLeftArrow, s_colorLeft);
    if(m_bRight)
        illuminateStrip(enumLightsRightArrow, s_colorRight);

    colorCorrect();
}
