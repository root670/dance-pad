#include "Panel.h"

void Panel::update() {
  m_sensorN.update();
  m_sensorE.update();
  m_sensorS.update();
  m_sensorW.update();
}

void Panel::calibrate() {
  m_sensorN.readSensor();
  m_sensorN.calibrate();
  m_sensorE.readSensor();
  m_sensorE.calibrate();
  m_sensorS.readSensor();
  m_sensorS.calibrate();
  m_sensorW.readSensor();
  m_sensorW.calibrate();
}

bool Panel::isPressed() const {
  return m_sensorN.isPressed() || m_sensorE.isPressed() ||
         m_sensorS.isPressed() || m_sensorW.isPressed();
}

// Get the north sensor corrected for the Arrow Panel PCB's orientation
const Sensor& Panel::getNorthSensor() const {
  switch (m_orientation) {
  default:
  case enumPanelOrientation0:
    return m_sensorN;
    break;
  case enumPanelOrientation90:
    return m_sensorE;
    break;
  case enumPanelOrientation180:
    return m_sensorS;
    break;
  case enumPanelOrientation270:
    return m_sensorW;
  }
}

// Get the east sensor corrected for the Arrow Panel PCB's orientation
const Sensor& Panel::getEastSensor() const {
  switch (m_orientation) {
  default:
  case enumPanelOrientation0:
    return m_sensorE;
    break;
  case enumPanelOrientation90:
    return m_sensorS;
    break;
  case enumPanelOrientation180:
    return m_sensorW;
    break;
  case enumPanelOrientation270:
    return m_sensorN;
  }
}

// Get the south sensor corrected for the Arrow Panel PCB's orientation
const Sensor& Panel::getSouthSensor() const {
  switch (m_orientation) {
  default:
  case enumPanelOrientation0:
    return m_sensorS;
    break;
  case enumPanelOrientation90:
    return m_sensorW;
    break;
  case enumPanelOrientation180:
    return m_sensorN;
    break;
  case enumPanelOrientation270:
    return m_sensorE;
  }
}

const Sensor& Panel::getWestSensor() const {
  switch (m_orientation) {
  default:
  case enumPanelOrientation0:
    return m_sensorW;
    break;
  case enumPanelOrientation90:
    return m_sensorN;
    break;
  case enumPanelOrientation180:
    return m_sensorE;
    break;
  case enumPanelOrientation270:
    return m_sensorS;
  }
}