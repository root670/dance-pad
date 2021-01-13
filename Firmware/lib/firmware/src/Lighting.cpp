#include <FastLED.h>
#include <OctoWS2811.h>

#include "Lighting.h"

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

namespace Lights
{

    void initialize()
    {
        FastLED.addLeds(&s_controller, s_leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
        FastLED.setBrightness(BRIGHTNESS);
    }

    bool bUp(false), bDown(false), bLeft(false), bRight(false);

    void illuminateStrip(lightIdentifier_t id, const CRGB &color)
    {
        for(int nLED = 0; nLED < NUM_LEDS_PER_STRIP; nLED++)
        {
            s_leds[(int)id*NUM_LEDS_PER_STRIP + nLED] = color;
        }
    }

    void setStatus(lightIdentifier_t id, bool bEnabled)
    {
        if(id == enumLightsUpArrow)
            bUp = bEnabled;
        else if(id == enumLightsDownArrow)
            bDown = bEnabled;
        else if(id == enumLightsLeftArrow)
            bLeft = bEnabled;
        else if(id == enumLightsRightArrow)
            bRight = bEnabled;
    }

    static CRGB s_colorBlue(0x18, 0, 0xff);
    static CRGB s_colorMag(0xeb, 0, 0x9b);

    void update()
    {
        if(bUp)
            illuminateStrip(enumLightsUpArrow, s_colorMag);
        if(bDown)
            illuminateStrip(enumLightsDownArrow, s_colorMag);
        if(bLeft)
            illuminateStrip(enumLightsLeftArrow, s_colorBlue);
        if(bRight)
            illuminateStrip(enumLightsRightArrow, s_colorBlue);

        fadeToBlackBy(s_leds, NUM_LEDS, 20);
    }

} // namespace Lights
