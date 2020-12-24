//
// Dance Pad Firmware
//
// * Auto-calibration of FSR sensors using a moving baseline.
// * Activate RBG LEDs in arrow PCBs based on SextetStream input.
// * Serial console for obtaining sensor data and reading/writing configuration.
//

#include <Arduino.h>
#include <array>
#include "Panel.h"

static String s_strVersion;
static char s_pSextetStream[14]; // Includes newline characteam

// Pins for sensors based on layout of the Dance Pad PCB
#define PIN_UP_N    A6
#define PIN_UP_E    A7
#define PIN_UP_S    A8
#define PIN_UP_W    A9
#define PIN_DOWN_N  A2
#define PIN_DOWN_E  A3
#define PIN_DOWN_S  A4
#define PIN_DOWN_W  A5
#define PIN_LEFT_N  A16
#define PIN_LEFT_E  A17
#define PIN_LEFT_S  A0
#define PIN_LEFT_W  A1
#define PIN_RIGHT_N A13
#define PIN_RIGHT_E A12
#define PIN_RIGHT_S A14
#define PIN_RIGHT_W A15

static Panel s_panelUp(enumPanelUp, enumPanelOrientation0, PIN_UP_N, PIN_UP_E, PIN_UP_S, PIN_UP_W);
static Panel s_panelDown(enumPanelDown, enumPanelOrientation270, PIN_DOWN_N, PIN_DOWN_E, PIN_DOWN_S, PIN_DOWN_W);
static Panel s_panelLeft(enumPanelLeft, enumPanelOrientation180, PIN_LEFT_N, PIN_LEFT_E, PIN_LEFT_S, PIN_LEFT_W);
static Panel s_panelRight(enumPanelRight, enumPanelOrientation0, PIN_RIGHT_N, PIN_RIGHT_E, PIN_RIGHT_S, PIN_RIGHT_W);

// Joystick button mapping
#define JOY_UP_BUTTON       1
#define JOY_DOWN_BUTTON     2
#define JOY_LEFT_BUTTON     3
#define JOY_RIGHT_BUTTON    4

// Force calibration of sensors in each panel
void calibratePanels()
{
    s_panelUp.calibrate();
    s_panelDown.calibrate();
    s_panelLeft.calibrate();
    s_panelRight.calibrate();
}

// Update sensor readings from each panel
void updatePanels()
{
    s_panelUp.update();
    s_panelDown.update();
    s_panelLeft.update();
    s_panelRight.update();
}

void setup()
{
    s_strVersion.concat("Dance Pad Firmware " __DATE__);
    s_strVersion.concat(' ');
    s_strVersion.concat(__TIME__);

    pinMode(LED_BUILTIN, OUTPUT);

    Serial.begin(9600);

    Joystick.useManualSend(true);
    Joystick.hat(-1);
    Joystick.X(512);
    Joystick.Y(512);
    Joystick.Z(512);
}

void printSensorValues()
{
    // name:value,name:value, ...
    Serial.print("Min:0,Max:1023,");
    Serial.print("UpN:");
    Serial.println(analogRead(PIN_UP_N));
    // Serial.print(analogRead(PIN_UP_N));
    // Serial.print(",");
    // Serial.print("UpE:");
    // Serial.print(analogRead(PIN_UP_E));
    // Serial.print(",");
    // Serial.print("UpS:");
    // Serial.print(analogRead(PIN_UP_S));
    // Serial.print(",");
    // Serial.print("UpW:");
    // Serial.print(analogRead(PIN_UP_W));
    // Serial.print(",");

    // Serial.print("DownN:");
    // Serial.print(analogRead(PIN_DOWN_N));
    // Serial.print(",");
    // Serial.print("DownE:");
    // Serial.print(analogRead(PIN_DOWN_E));
    // Serial.print(",");
    // Serial.print("DownS:");
    // Serial.print(analogRead(PIN_DOWN_S));
    // Serial.print(",");
    // Serial.print("DownW:");
    // Serial.print(analogRead(PIN_DOWN_W));
    // Serial.print(",");

    // Serial.print("LeftN:");
    // Serial.print(analogRead(PIN_LEFT_N));
    // Serial.print(",");
    // Serial.print("LeftE:");
    // Serial.print(analogRead(PIN_LEFT_E));
    // Serial.print(",");
    // Serial.print("LeftS:");
    // Serial.print(analogRead(PIN_LEFT_S));
    // Serial.print(",");
    // Serial.print("LeftW:");
    // Serial.print(analogRead(PIN_LEFT_W));
    // Serial.print(",");

    // Serial.print("RightN:");
    // Serial.print(analogRead(PIN_RIGHT_N));
    // Serial.print(",");
    // Serial.print("RightE:");
    // Serial.print(analogRead(PIN_RIGHT_E));
    // Serial.print(",");
    // Serial.print("RightS:");
    // Serial.print(analogRead(PIN_RIGHT_S));
    // Serial.print(",");
    // Serial.print("RightW:");
    // Serial.println(analogRead(PIN_RIGHT_W));
}

// void decodeSextetStream()
// {
//     // Cabinet
//     g_pSextetStream[0] & 0x01; // Marquee upper-left
//     g_pSextetStream[0] & 0x02; // Marquee upper-right
//     g_pSextetStream[0] & 0x04; // Marquee lower-left
//     g_pSextetStream[0] & 0x08; // Marquee lower-right
//     g_pSextetStream[0] & 0x10; // Bass left
//     g_pSextetStream[0] & 0x20; // Bass right

//     // Player 1
//     g_pSextetStream[1] & 0x01; // P1 Menu Left
//     g_pSextetStream[1] & 0x02; // P1 Menu Right
//     g_pSextetStream[1] & 0x04; // P1 Menu Up
//     g_pSextetStream[1] & 0x08; // P1 Menu Down
//     g_pSextetStream[1] & 0x10; // P1 Start
//     g_pSextetStream[1] & 0x20; // P1 Select
//     g_pSextetStream[2] & 0x01; // P1 Back
//     g_pSextetStream[2] & 0x02; // P1 Coin
//     g_pSextetStream[2] & 0x04; // P1 Operator
//     g_pSextetStream[2] & 0x08; // P1 Effect Up
//     g_pSextetStream[2] & 0x10; // P1 Effect Down
//     g_pSextetStream[3] & 0x01; // P1 Menu Left
//     g_pSextetStream[3] & 0x02; // P1 Menu Right
//     g_pSextetStream[3] & 0x04; // P1 Menu Up
//     g_pSextetStream[3] & 0x08; // P1 Menu Down
//     g_pSextetStream[3] & 0x10; // P1 Start
//     g_pSextetStream[3] & 0x20; // P1 Select
//     g_pSextetStream[4] & 0x01; // P1 Pad Left
//     g_pSextetStream[4] & 0x02; // P1 Pad Right
//     g_pSextetStream[4] & 0x04; // P1 Pad Up
//     g_pSextetStream[4] & 0x08; // P1 Pad Down
// }

// Process data sent over the serial connection
//
// Input data can either be lighting data in SextetStream format or a command.
// When the input is lighting data, no response is sent. All commands are
// prefixed with `-` and have a single-line response terminated with a newline
// character (`\n`).
class SerialProcessor
{
public:
    SerialProcessor()
    {
        m_strCommand.reserve(32);
        m_strResponse.reserve(1024);
    }

    static const String kCmdVersion;
    static const String kCmdBlink;
    static const String kCmdConfig;
    static const String kCmdValues;

    void update()
    {
    if(Serial.available())
    {
        char c = Serial.read();
        if(c >= 0x30 && c <= 0x6F)
        {
            // Light state. Read remaining 13 bytes of stream
            s_pSextetStream[0] = c;
            Serial.readBytes(s_pSextetStream+1, 13);

            // Update the lights
            //decodeSextetStream();
        }
        else if(c == '-')
        {
            // Command terminated by newline
                m_strCommand = Serial.readStringUntil('\n');
                m_strCommand.trim();
                m_strResponse = "";

                if(m_strCommand.equalsIgnoreCase(kCmdVersion))
            {
                    m_strResponse = s_strVersion;
            }
                else if(m_strCommand.equalsIgnoreCase(kCmdBlink))
            {
                // Blink the builtin LED.
                digitalWrite(LED_BUILTIN, HIGH);
                delay(100);
                digitalWrite(LED_BUILTIN, LOW);
                delay(100);
                digitalWrite(LED_BUILTIN, HIGH);
                delay(100);
                digitalWrite(LED_BUILTIN, LOW);
            }
                else if(m_strCommand.equalsIgnoreCase(kCmdConfig))
            {
                    // Get the configuration of the panels
                    m_strResponse.append(s_panelUp.m_orientation);
                    m_strResponse.append(',');
                    m_strResponse.append(s_panelDown.m_orientation);
                    m_strResponse.append(',');
                    m_strResponse.append(s_panelLeft.m_orientation);
                    m_strResponse.append(',');
                    m_strResponse.append(s_panelRight.m_orientation);
                    m_strResponse.append(',');

                    // Other config items
                    // m_strResponse.append()
            }
                else if(m_strCommand.equalsIgnoreCase(kCmdValues))
                {
                    // Get the raw values and thresholds for each sensor
                    for(auto const &panel : {
                        s_panelUp,
                        s_panelDown,
                        s_panelLeft,
                        s_panelRight
                    })
                    {
                        for(auto const &sensor : {
                            panel.getNorthSensor(),
                            panel.getEastSensor(),
                            panel.getSouthSensor(),
                            panel.getWestSensor()
                        })
            {
                            m_strResponse.append(sensor.m_nPressure);
                            m_strResponse.append(',');
                            m_strResponse.append(sensor.m_nTriggerThreshold);
                            m_strResponse.append(',');
                            m_strResponse.append(sensor.m_nReleaseThreshold);
                            m_strResponse.append(',');
                        }
                    }

                    // Remove trailing comma
                    m_strResponse.setCharAt(m_strResponse.length() - 1, '\0');
            }
            else
            {
                    m_strResponse = "Unknown command";
            }

                Serial.println(m_strResponse);
        }
    }
}

private:

    String m_strCommand, m_strResponse;
};

const String SerialProcessor::kCmdVersion   = "version";
const String SerialProcessor::kCmdBlink     = "blink";
const String SerialProcessor::kCmdConfig    = "config";
const String SerialProcessor::kCmdValues    = "v";

static SerialProcessor s_serialProcessor;

void updateJoystick()
{
    Joystick.button(JOY_UP_BUTTON, s_panelUp.isPressed());
    Joystick.button(JOY_DOWN_BUTTON, s_panelUp.isPressed());
    Joystick.button(JOY_LEFT_BUTTON, s_panelUp.isPressed());
    Joystick.button(JOY_RIGHT_BUTTON, s_panelUp.isPressed());
    Joystick.send_now();
}

static elapsedMicros s_timeSinceJoystickUpdate;
static elapsedMicros s_timeSinceLEDUpdate;

const uint32_t kMicrosPerSecond = 1000000;
const uint32_t kJoystickUpdateFrequency = 1000;
const uint32_t kLEDUpdateFrequency = 60;

void loop()
{
    updatePanels();
    s_serialProcessor.update();

    // Limit frequency of joystick reports
    if(s_timeSinceJoystickUpdate >= kMicrosPerSecond/kJoystickUpdateFrequency)
    {
        s_timeSinceJoystickUpdate -= kMicrosPerSecond/kJoystickUpdateFrequency;
        updateJoystick();
    }

    // // Limit frequency of LEDs
    // if(s_timeSinceLEDUpdate >= kMicrosPerSecond/kLEDUpdateFrequency)
    // {
    //     s_timeSinceLEDUpdate -= kMicrosPerSecond/kLEDUpdateFrequency;
    //     FastLED.show();
    // }

    //printSensorValues();
}