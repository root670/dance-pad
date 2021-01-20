#include <FastLED.h>
#include <OctoWS2811.h>

#include "Lighting.h"
#include "Config.h"

#define NUM_LEDS_PER_STRIP  25
#define NUM_STRIPS          4
#define NUM_LEDS            NUM_LEDS_PER_STRIP * NUM_STRIPS
#define BRIGHTNESS          150
#define COLOR_ORDER         GRB

static CRGB s_leds[NUM_LEDS];

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
    FastLED.addLeds(&s_controller, s_leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
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
        s_leds[(int)id*NUM_LEDS_PER_STRIP + nLED] = color;
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

void Lights::update()
{
    fadeToBlackBy(s_leds, NUM_LEDS, 20);

    if(m_bUp)
        illuminateStrip(enumLightsUpArrow, s_colorUp);
    if(m_bDown)
        illuminateStrip(enumLightsDownArrow, s_colorDown);
    if(m_bLeft)
        illuminateStrip(enumLightsLeftArrow, s_colorLeft);
    if(m_bRight)
        illuminateStrip(enumLightsRightArrow, s_colorRight);
}
