# Arrow Panel PCB

Board placed under each panel. Houses 25 WS2812B V5 RGB LEDs and connects to 4
FSR sensors using JST-XE or JST-ZH connectors. Connects back to the Main PCB
using a CAT6 cable for the sensors and cables secured to a screw terminal for
the lights.

## Notes

* The pull down resistor used for each sensor may need to be adjusted depending
  on the sensors being used.
* The board requires the "V5" revision of the WS2812B LEDs. The advantage of
  this version is that have an internal capacitor so you don't the need to place
  an external capacitor between the positive voltage and ground pins. If an
  older version is used the LEDs will likely be damaged when they receive power.

## Issues

* I was unable to get the LEDs to work reliably using multiple 3.3v and 5v
  microcontrollers and WS2812B libraries. I believe there is something wrong
  with my design or the LEDs sourced by LCSC were not actually the 5V
  variations. LCSC has been unable to confirm if they are actually the V5
  version.
* The PCB is too big to fit within the frame underneath the panels when using
  corner brackets. As a workaround the PCBs need to be placed underneath the
  frame.
