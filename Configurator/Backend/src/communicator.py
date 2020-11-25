"""Communicate with the board firmware.
"""

from enum import Enum
from typing import Mapping
import serial


class PanelOrientation(Enum):
    """Orientation of the Arrow Panel PCB within an panel. Based on
    counterclockwise rotation.
    """

    Standard = 0
    Rotated90Degrees = 90
    Rotated180Degrees = 180
    Rotated270Degrees = 270

    @staticmethod
    def from_degrees(degrees: int) -> Enum:
        """Get PanelOrientation enumeration based on degrees.
        """
        try:
            return {
                0: PanelOrientation.Standard,
                90: PanelOrientation.Rotated90Degrees,
                180: PanelOrientation.Rotated180Degrees,
                270: PanelOrientation.Rotated270Degrees,
            }[degrees]
        except KeyError:
            raise KeyError(f'Invalid panel orientation: {degrees}')


class Communicator:
    """Communicate with the board firmware.
    """

    COMMAND_VERSION = 'version'
    COMMAND_BLINK = 'blink'
    COMMAND_PANELCONFIG = 'panelconfig'

    def __init__(
        self,
        ser=serial.Serial('/dev/cu.usbmodem60430801', timeout=1)
    ):
        """Communicate with the firmware using a serial interface.

        Args:
            ser: Serial object
        """
        self._ser = ser

    def __send_command(self, command: str) -> None:
        """Send command to the device.

        Args:
            command: Command string.
        """
        self._ser.write(f'-{command}\n'.encode('ascii'))

    def __get_line(self) -> str:
        """Get ASCII-encoded line from the device.
        """
        return self._ser.readline().decode('ascii').strip()

    def get_version(self) -> str:
        """Get firmware version string.
        """
        self.__send_command(self.COMMAND_VERSION)
        return self.__get_line()

    def blink_led(self) -> None:
        """Blink the on-board LED on the Teensy board.
        """
        self.__send_command(self.COMMAND_BLINK)

    def get_panel_config(self) -> Mapping[str, Enum]:
        """Get configuration of panels.

        Returns:
            Dictionary of configurations for each panel.
        """
        self.__send_command(self.COMMAND_PANELCONFIG)
        response = self.__get_line()
        split = response.split(',')
        return dict(
            up=PanelOrientation.from_degrees(int(split[0])),
            down=PanelOrientation.from_degrees(int(split[1])),
            left=PanelOrientation.from_degrees(int(split[2])),
            right=PanelOrientation.from_degrees(int(split[3])),
        )

    def get_sensor_values(self) -> Mapping[str, Mapping[str, int]]:
        """Get current raw sensor values.

        Returns:
            Dictionary with mapping an arrow direction to dictionary of values for each sensor.
        """
        self.__send_command('v')
        values = self.__get_line().split(',')
        return dict(
            up=dict(
                north=int(values.pop(0)),
                east=int(values.pop(0)),
                south=int(values.pop(0)),
                west=int(values.pop(0)),
            ),
            down=dict(
                north=int(values.pop(0)),
                east=int(values.pop(0)),
                south=int(values.pop(0)),
                west=int(values.pop(0)),
            ),
            left=dict(
                north=int(values.pop(0)),
                east=int(values.pop(0)),
                south=int(values.pop(0)),
                west=int(values.pop(0)),
            ),
            right=dict(
                north=int(values.pop(0)),
                east=int(values.pop(0)),
                south=int(values.pop(0)),
                west=int(values.pop(0)),
            ),
        )
