# Main PCB

Breakout board for a Teensy 4.1 with 4 RJ45 connectors to connect the sensors
from each arrow and 5 3-pin screw terminals to connect the lights. An additional
3-pin screw terminal is available for an auxilary lighting strip.

## Notes

* At least a 6A 5V DC power supply is needed to support displaying all LEDs at
  maximum brightness.

## Issues

* The LED pins for the four arrows  don't allow for "parallel output" with
  FastLED or WS2812Serial because I didn't realize only a subset of pins support
  that feature. The AuxLED pin however should work with parallel output.
* Silkscreen labels are missing for most connectors.
