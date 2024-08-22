#pragma once

#include "Lighting.h"
#include "Sensor.h"

// Orientation of the Arrow Panel PCB used for a panel.
//
// We assume all Arrow Panel PCBs are oriented with the arrow symbol facing the
// front of the pad by default. It may be preferable to rotate the PCB to make
// wire management easier. The orientation can be denoted to ensure absolute
// N/E/S/W location of each sensor can be determined. All rotations are
// counterclockwise.
typedef enum {
  enumPanelOrientation0,
  enumPanelOrientation90 = 90,
  enumPanelOrientation180 = 180,
  enumPanelOrientation270 = 270
} enumPanelOrientation;

// Panel location in the pad's frame
typedef enum {
  enumPanelUpperLeft,
  enumPanelUp,
  enumPanelUpperRight,
  enumPanelLeft,
  enumPanelCenter,
  enumPanelRight,
  enumPanelLowerLeft,
  enumPanelDown,
  enumPanelLowerRight
} enumPanelType;

// Panel with 4 cardinal sensors
class Panel {
public:
  Panel(
    enumPanelType type,
    enumPanelOrientation orientation,
    int nPinN,
    int nPinE,
    int nPinS,
    int nPinW)
      : m_orientation(orientation), m_sensorN(nPinN), m_sensorE(nPinE),
        m_sensorS(nPinS), m_sensorW(nPinW) {}

  // Update state of the panel
  void update();

  // Force calibration of all sensors
  void calibrate();

  // Are any of the sensors currently pressed?
  bool isPressed() const;

  // Get the north sensor corrected for the Arrow Panel PCB's orientation
  const Sensor& getNorthSensor() const;

  // Get the east sensor corrected for the Arrow Panel PCB's orientation
  const Sensor& getEastSensor() const;

  // Get the south sensor corrected for the Arrow Panel PCB's orientation
  const Sensor& getSouthSensor() const;

  // Get the west sensor corrected for the Arrow Panel PCB's orientation
  const Sensor& getWestSensor() const;

  enumPanelType m_type;
  enumPanelOrientation m_orientation;
  Sensor m_sensorN;
  Sensor m_sensorE;
  Sensor m_sensorS;
  Sensor m_sensorW;
};
