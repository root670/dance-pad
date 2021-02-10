# Supplemental

Various helper scripts and files related to this project.

## Files

* `lights_bridge.py`: Python script to send output messages for lights from MAME
  to serial devices as a SextetStream. To use this, launch MAME with `-output
  network` or change the `output` setting in `mame.ini` to `network`.

* `ddr.lua`: Lua script for MAME that disables frame skip during the
  initialization sequence of DDR titles. To use this, launch MAME with
  `autoboot_script ddr.lua`.
