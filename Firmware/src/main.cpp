//
// Dance Pad Firmware
//
// * Auto-calibration of FSR sensors using a moving baseline.
// * Activate RBG LEDs in arrow PCBs based on SextetStream input.
// * Serial console for obtaining sensor data and reading/writing configuration.
//
#include <Arduino.h>
#include <Keyboard.h>
#include <array>

#include "Config.h"
#include "Lighting.h"
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

static Panel s_panelUp(
  enumPanelUp, enumPanelOrientation0, PIN_UP_N, PIN_UP_E, PIN_UP_S, PIN_UP_W);
static Panel s_panelDown(
  enumPanelDown,
  enumPanelOrientation270,
  PIN_DOWN_N,
  PIN_DOWN_E,
  PIN_DOWN_S,
  PIN_DOWN_W);
static Panel s_panelLeft(
  enumPanelLeft,
  enumPanelOrientation270,
  PIN_LEFT_N,
  PIN_LEFT_E,
  PIN_LEFT_S,
  PIN_LEFT_W);
static Panel s_panelRight(
  enumPanelRight,
  enumPanelOrientation0,
  PIN_RIGHT_N,
  PIN_RIGHT_E,
  PIN_RIGHT_S,
  PIN_RIGHT_W);

// Joystick button mapping
#define JOY_UP_BUTTON    1
#define JOY_DOWN_BUTTON  2
#define JOY_LEFT_BUTTON  3
#define JOY_RIGHT_BUTTON 4

// Scheduling
const uint32_t kJoystickUpdateFrequency = 1000;
const uint32_t kLEDUpdateFrequency = 100;

// Force calibration of sensors in each panel
void calibratePanels() {
  s_panelUp.calibrate();
  s_panelDown.calibrate();
  s_panelLeft.calibrate();
  s_panelRight.calibrate();
}

// Update sensor readings from each panel
void updatePanels() {
  s_panelUp.update();
  s_panelDown.update();
  s_panelLeft.update();
  s_panelRight.update();
}

void setup() {
  delay(1000);
  s_strVersion.concat("Dance Pad Firmware " __DATE__);
  s_strVersion.concat(' ');
  s_strVersion.concat(__TIME__);

  pinMode(LED_BUILTIN, OUTPUT);

  Configuration::getInstance()->read();

  Serial.begin(9600);

  calibratePanels();

  // Joystick.useManualSend(true);
  // Joystick.hat(-1);
  // Joystick.X(512);
  // Joystick.Y(512);
  // Joystick.Z(512);

  for (int i = 0; i < 3; i++) {
    CRGB color(i == 0 ? 255 : 0, i == 1 ? 255 : 0, i == 2 ? 255 : 0);
    Lights::getInstance()->illuminateStrip(enumLightsUpArrow, color);
    Lights::getInstance()->illuminateStrip(enumLightsDownArrow, color);
    Lights::getInstance()->illuminateStrip(enumLightsLeftArrow, color);
    Lights::getInstance()->illuminateStrip(enumLightsRightArrow, color);
    Lights::getInstance()->update();
    FastLED.show();
    FastLED.delay(200);
  }
}

void printSensorValues() {
  // name:value,name:value, ...
  Serial.print("Min:0,Max:1023,");
  Serial.print("UpN:");
  // Serial.println(analogRead(PIN_UP_N));
  Serial.print(analogRead(PIN_UP_N));
  Serial.print(",");
  Serial.print("UpE:");
  Serial.print(analogRead(PIN_UP_E));
  Serial.print(",");
  Serial.print("UpS:");
  Serial.print(analogRead(PIN_UP_S));
  Serial.print(",");
  Serial.print("UpW:");
  Serial.print(analogRead(PIN_UP_W));
  Serial.print(",");

  Serial.print("DownN:");
  Serial.print(analogRead(PIN_DOWN_N));
  Serial.print(",");
  Serial.print("DownE:");
  Serial.print(analogRead(PIN_DOWN_E));
  Serial.print(",");
  Serial.print("DownS:");
  Serial.print(analogRead(PIN_DOWN_S));
  Serial.print(",");
  Serial.print("DownW:");
  Serial.print(analogRead(PIN_DOWN_W));
  Serial.print(",");

  Serial.print("LeftN:");
  Serial.print(analogRead(PIN_LEFT_N));
  Serial.print(",");
  Serial.print("LeftE:");
  Serial.print(analogRead(PIN_LEFT_E));
  Serial.print(",");
  Serial.print("LeftS:");
  Serial.print(analogRead(PIN_LEFT_S));
  Serial.print(",");
  Serial.print("LeftW:");
  Serial.print(analogRead(PIN_LEFT_W));
  Serial.print(",");

  Serial.print("RightN:");
  Serial.print(analogRead(PIN_RIGHT_N));
  Serial.print(",");
  Serial.print("RightE:");
  Serial.print(analogRead(PIN_RIGHT_E));
  Serial.print(",");
  Serial.print("RightS:");
  Serial.print(analogRead(PIN_RIGHT_S));
  Serial.print(",");
  Serial.print("RightW:");
  Serial.println(analogRead(PIN_RIGHT_W));
}

void decodeSextetStream() {
  // Player 1 pad lights
  Lights::getInstance()->setStatus(
    enumLightsLeftArrow, s_pSextetStream[3] & 0x01);
  Lights::getInstance()->setStatus(
    enumLightsRightArrow, s_pSextetStream[3] & 0x02);
  Lights::getInstance()->setStatus(
    enumLightsUpArrow, s_pSextetStream[3] & 0x04);
  Lights::getInstance()->setStatus(
    enumLightsDownArrow, s_pSextetStream[3] & 0x08);
}

// Process data sent over the serial connection
//
// Input data can either be lighting data in SextetStream format or a command.
// When the input is lighting data, no response is sent. All commands are
// prefixed with `-` and terminated with a newline character. Commands may have
// a single-line response terminated with a newline character (`\n`).
class SerialProcessor {
public:
  SerialProcessor() {
    m_strCommand.reserve(32);
    m_strResponse.reserve(1024);
  }

  void update() {
    if (Serial.available()) {
      char c = Serial.read();
      if (c >= 0x30 && c <= 0x6F) {
        // Light state. Read remaining 13 bytes of stream
        s_pSextetStream[0] = c;
        Serial.readBytes(s_pSextetStream + 1, 13);

        // Update the lights
        decodeSextetStream();
      } else if (c == '-') {
        // Command terminated by newline
        m_strCommand = Serial.readStringUntil('\n');
        m_strCommand.trim();
        m_strResponse = "";

        if (m_strCommand.equalsIgnoreCase(kCmdVersion)) {
          onCommandVersion();
        } else if (m_strCommand.equalsIgnoreCase(kCmdBlink)) {
          onCommandBlink();
        } else if (m_strCommand.equalsIgnoreCase(kCmdGetConfig)) {
          onCommandGetConfig();
        } else if (m_strCommand.equalsIgnoreCase(kCmdSetConfig)) {
          onCommandSetConfig();
        } else if (m_strCommand.equalsIgnoreCase(kCmdPersist)) {
          onCommandPersist();
        } else if (m_strCommand.equalsIgnoreCase(kCmdReset)) {
          onCommandReset();
        } else if (m_strCommand.equalsIgnoreCase(kCmdValues)) {
          onCommandGetValues();
        } else if (m_strCommand.equalsIgnoreCase(kCmdCalibrate)) {
          calibratePanels();
        } else {
          m_strResponse = "Unknown command";
        }

        if (m_strResponse.length() > 0) {
          Serial.println(m_strResponse);
        }
      }
    }
  }

private:
  static const String kCmdVersion;
  static const String kCmdBlink;
  static const String kCmdGetConfig;
  static const String kCmdSetConfig;
  static const String kCmdPersist;
  static const String kCmdReset;
  static const String kCmdValues;
  static const String kCmdCalibrate;

  static const String kConfigTypeStr;
  static const String kConfigTypeUInt16;
  static const String kConfigTypeUInt32;

  static const String kResponseSuccess;
  static const String kResponseFailure;

  // Get the version
  void onCommandVersion() { m_strResponse = s_strVersion; }

  // Blink the builtin LED.
  void onCommandBlink() {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(100);
    digitalWrite(LED_BUILTIN, LOW);
    delay(100);
    digitalWrite(LED_BUILTIN, HIGH);
    delay(100);
    digitalWrite(LED_BUILTIN, LOW);
  }

  // Get configuration values
  void onCommandGetConfig() {
    // Get the configuration of the panels
    // Up
    m_strResponse.append(s_panelUp.m_orientation);
    m_strResponse.append(',');
    m_strResponse.append(s_panelUp.getNorthSensor().getPin());
    m_strResponse.append(',');
    m_strResponse.append(s_panelUp.getEastSensor().getPin());
    m_strResponse.append(',');
    m_strResponse.append(s_panelUp.getSouthSensor().getPin());
    m_strResponse.append(',');
    m_strResponse.append(s_panelUp.getWestSensor().getPin());
    m_strResponse.append(',');

    // Down
    m_strResponse.append(s_panelDown.m_orientation);
    m_strResponse.append(',');
    m_strResponse.append(s_panelDown.getNorthSensor().getPin());
    m_strResponse.append(',');
    m_strResponse.append(s_panelDown.getEastSensor().getPin());
    m_strResponse.append(',');
    m_strResponse.append(s_panelDown.getSouthSensor().getPin());
    m_strResponse.append(',');
    m_strResponse.append(s_panelDown.getWestSensor().getPin());
    m_strResponse.append(',');

    // Left
    m_strResponse.append(s_panelLeft.m_orientation);
    m_strResponse.append(',');
    m_strResponse.append(s_panelLeft.getNorthSensor().getPin());
    m_strResponse.append(',');
    m_strResponse.append(s_panelLeft.getEastSensor().getPin());
    m_strResponse.append(',');
    m_strResponse.append(s_panelLeft.getSouthSensor().getPin());
    m_strResponse.append(',');
    m_strResponse.append(s_panelLeft.getWestSensor().getPin());
    m_strResponse.append(',');

    // Right
    m_strResponse.append(s_panelRight.m_orientation);
    m_strResponse.append(',');
    m_strResponse.append(s_panelRight.getNorthSensor().getPin());
    m_strResponse.append(',');
    m_strResponse.append(s_panelRight.getEastSensor().getPin());
    m_strResponse.append(',');
    m_strResponse.append(s_panelRight.getSouthSensor().getPin());
    m_strResponse.append(',');
    m_strResponse.append(s_panelRight.getWestSensor().getPin());
    m_strResponse.append(',');

    // Other config items
    m_strResponse.append(Configuration::getInstance()->toString());

    if (m_strResponse[m_strResponse.length() - 1] == ',') {
      m_strResponse.remove(m_strResponse.length() - 1);
    }
  }

  // Set a configuration value. The sender must provide an additional line:
  // `TYPE KEY=VALUE\n`, where `TYPE` is `str`, `u16`, or `u32`.
  void onCommandSetConfig() {
    String strType = Serial.readStringUntil(' ');
    String strKey = Serial.readStringUntil('=');
    String strValue = Serial.readStringUntil('\n');
    if (strType.equalsIgnoreCase(kConfigTypeStr)) {
      Configuration::getInstance()->setString(strKey, strValue);
      m_strResponse = kResponseSuccess;
    } else if (strType.equalsIgnoreCase(kConfigTypeUInt16)) {
      Configuration::getInstance()->setUInt16(strKey, atoi(strValue.c_str()));
      m_strResponse = kResponseSuccess;
    } else if (strType.equalsIgnoreCase(kConfigTypeUInt32)) {
      Configuration::getInstance()->setUInt32(strKey, atoi(strValue.c_str()));
      m_strResponse = kResponseSuccess;
    } else {
      m_strResponse = kResponseFailure;
    }
  }

  // Save configuration items to EEPROM
  void onCommandPersist() { Configuration::getInstance()->write(); }

  // Reset configuration items in memory
  void onCommandReset() { Configuration::getInstance()->reset(); }

  // Get the raw values and thresholds for each sensor
  void onCommandGetValues() {
    for (auto const& panel :
         {s_panelUp, s_panelDown, s_panelLeft, s_panelRight}) {
      for (auto const& sensor :
           {panel.getNorthSensor(),
            panel.getEastSensor(),
            panel.getSouthSensor(),
            panel.getWestSensor()}) {
        m_strResponse.append(sensor.getPressure());
        m_strResponse.append(',');
        m_strResponse.append(sensor.getTriggerThreshold());
        m_strResponse.append(',');
        m_strResponse.append(sensor.getReleaseThreshold());
        m_strResponse.append(',');
      }
    }

    // Remove trailing comma
    m_strResponse.remove(m_strResponse.length() - 1);
  }

  String m_strCommand, m_strResponse;
};

const String SerialProcessor::kCmdVersion = "version";
const String SerialProcessor::kCmdBlink = "blink";
const String SerialProcessor::kCmdGetConfig = "config";
const String SerialProcessor::kCmdSetConfig = "set";
const String SerialProcessor::kCmdPersist = "persist";
const String SerialProcessor::kCmdReset = "reset";
const String SerialProcessor::kCmdValues = "v";
const String SerialProcessor::kCmdCalibrate = "calibrate";

const String SerialProcessor::kResponseSuccess = "!";
const String SerialProcessor::kResponseFailure = "?";

const String SerialProcessor::kConfigTypeStr = "str";
const String SerialProcessor::kConfigTypeUInt16 = "u16";
const String SerialProcessor::kConfigTypeUInt32 = "u32";

static SerialProcessor s_serialProcessor;

// void updateJoystick()
// {
//     Joystick.button(JOY_UP_BUTTON, s_panelUp.isPressed());
//     Joystick.button(JOY_DOWN_BUTTON, s_panelDown.isPressed());
//     Joystick.button(JOY_LEFT_BUTTON, s_panelLeft.isPressed());
//     Joystick.button(JOY_RIGHT_BUTTON, s_panelRight.isPressed());
//     Joystick.send_now();
// }

void updateKeyboard() {
  if (s_panelUp.isPressed()) {
    Keyboard.press('w');
  } else {
    Keyboard.release('w');
  }
  if (s_panelDown.isPressed()) {
    Keyboard.press('s');
  } else {
    Keyboard.release('s');
  }
  if (s_panelLeft.isPressed()) {
    Keyboard.press('a');
  } else {
    Keyboard.release('a');
  }
  if (s_panelRight.isPressed()) {
    Keyboard.press('d');
  } else {
    Keyboard.release('d');
  }
  Keyboard.send_now();
}

static elapsedMicros s_timeSinceJoystickUpdate;
static elapsedMicros s_timeSinceLEDUpdate;

const uint32_t kMicrosPerSecond = 1000000;

static const String kAutoLights("auto_lights");

void loop() {
  // It takes about 17us to sample each sensor (taking the average of 4
  // readings within the analogRead() call), so with 16 sensors we can read
  // all panels about 3 times and still have about 150-200us left for lights,
  // so read intervals should be very consistent.
  updatePanels();
  s_serialProcessor.update();

  // Limit frequency of joystick reports
  if (
    s_timeSinceJoystickUpdate >= kMicrosPerSecond / kJoystickUpdateFrequency) {
    s_timeSinceJoystickUpdate -= kMicrosPerSecond / kJoystickUpdateFrequency;
    // updateJoystick();
    updateKeyboard();
  }

  // Limit frequency of LEDs
  if (s_timeSinceLEDUpdate >= kMicrosPerSecond / kLEDUpdateFrequency) {
    s_timeSinceLEDUpdate -= kMicrosPerSecond / kLEDUpdateFrequency;
    if (Configuration::getInstance()->getUInt16(kAutoLights, 0) > 0) {
      Lights::getInstance()->setStatus(
        enumLightsLeftArrow, s_panelLeft.isPressed());
      Lights::getInstance()->setStatus(
        enumLightsRightArrow, s_panelRight.isPressed());
      Lights::getInstance()->setStatus(
        enumLightsUpArrow, s_panelUp.isPressed());
      Lights::getInstance()->setStatus(
        enumLightsDownArrow, s_panelDown.isPressed());
    }
    Lights::getInstance()->update();
    FastLED.show();
  }

  // printSensorValues();
}