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
    COMMAND_GETCONFIG = 'config'
    COMMAND_SETCONFIG = 'set'
    COMMAND_PERSIST = 'persist'
    COMMAND_VALUES = 'v'

    RESPONSE_SUCCESS = '!'
    RESPONSE_FAILURE = '?'

    def __init__(
        self,
        ser: serial.Serial
    ) -> None:
        """Communicate with the firmware using a serial interface.

        Args:
            ser: Serial object
        """
        self._ser = ser

    def __send_line(self, line: str) -> None:
        """Send line terminated with newline character.

        Args:
            line: The line to send.
        """
        self._ser.write(f'{line}\n'.encode('ascii'))


    def __get_line(self) -> str:
        """Get ASCII-encoded line from the device.
        """
        return self._ser.readline().decode('ascii').strip()

    def __send_command(self, command: str) -> None:
        """Send command to the device.

        Args:
            command: Command string.
        """
        self.__send_line(f'-{command}')

    def __set_u16_config(self, key: str, value) -> bool:
        """Set a configuration item for a 16-bit unsigned value.

        Args:
            key: Name of configuration item.
            value: The value to set the configuration item to.

        Returns:
            True if the value was set successfully. Otherwise, False.
        """
        self.__send_command(self.COMMAND_SETCONFIG)
        self.__send_line(f'u16 {key}={value}')
        response = self.__get_line()

        if not response in {
            Communicator.RESPONSE_SUCCESS,
            Communicator.RESPONSE_FAILURE
        }:
            raise ValueError(f'Unexpected response: {response}')

        return response == self.RESPONSE_SUCCESS

    def get_version(self) -> str:
        """Get firmware version string.
        """
        self.__send_command(self.COMMAND_VERSION)
        return self.__get_line()

    def blink_led(self) -> None:
        """Blink the on-board LED on the Teensy board.
        """
        self.__send_command(self.COMMAND_BLINK)

    def set_thresholds(self, pin: int, trigger: int, release:int) -> bool:
        """Set the tresholds for a sensor.

        Args:
            pin: Pin on the Teensy connected to the sensor.
            trigger: Value above baseline to trigger a hit.
            release: Value above baseline to trigger a release.

        Returns:
            True if the threshold was set successfully. Otherwise, False.
        """
        if not 0 < trigger < 1024:
            raise ValueError('`trigger` must be in range [0, 1024).')
        if not 0 < release < 1024:
            raise ValueError('`release` must be in range [0, 1024).')
        if release > trigger:
            raise ValueError(
                '`trigger` must be greater than or equal to `release`'
            )

        key_name = f'sensor{pin}'

        return self.__set_u16_config(key_name + 'trigger', trigger) and \
            self.__set_u16_config(key_name + 'release', release)

    def get_config(self) -> Mapping[str, Enum]:
        """Get configuration.

        Returns:
            Dictionary of configuration items.
        """
        self.__send_command(self.COMMAND_GETCONFIG)
        response = self.__get_line()
        split = response.split(',')
        return dict(
            up=dict(
                orientation=PanelOrientation.from_degrees(int(split.pop(0))),
                north_pin=int(split.pop(0)),
                east_pin=int(split.pop(0)),
                south_pin=int(split.pop(0)),
                west_pin=int(split.pop(0)),
            ),
            down=dict(
                orientation=PanelOrientation.from_degrees(int(split.pop(0))),
                north_pin=int(split.pop(0)),
                east_pin=int(split.pop(0)),
                south_pin=int(split.pop(0)),
                west_pin=int(split.pop(0)),
            ),
            left=dict(
                orientation=PanelOrientation.from_degrees(int(split.pop(0))),
                north_pin=int(split.pop(0)),
                east_pin=int(split.pop(0)),
                south_pin=int(split.pop(0)),
                west_pin=int(split.pop(0)),
            ),
            right=dict(
                orientation=PanelOrientation.from_degrees(int(split.pop(0))),
                north_pin=int(split.pop(0)),
                east_pin=int(split.pop(0)),
                south_pin=int(split.pop(0)),
                west_pin=int(split.pop(0)),
            ),
        )

    def get_sensor_values(self) -> Mapping[str, Mapping[str, int]]:
        """Get current raw sensor values.

        Returns:
            Nested dictionary mapping arrow directions to cardinal
            direction sensors with `value`, `trigger_threshold`, and
            `release_threshold` values.
        """
        self.__send_command(self.COMMAND_VALUES)
        values = self.__get_line().split(',')
        return dict(
            up=dict(
                north=dict(
                    value=int(values.pop(0)),
                    trigger_threshold=int(values.pop(0)),
                    release_threshold=int(values.pop(0)),
                ),
                east=dict(
                    value=int(values.pop(0)),
                    trigger_threshold=int(values.pop(0)),
                    release_threshold=int(values.pop(0)),
                ),
                south=dict(
                    value=int(values.pop(0)),
                    trigger_threshold=int(values.pop(0)),
                    release_threshold=int(values.pop(0)),
                ),
                west=dict(
                    value=int(values.pop(0)),
                    trigger_threshold=int(values.pop(0)),
                    release_threshold=int(values.pop(0)),
                ),
            ),
            down=dict(
                north=dict(
                    value=int(values.pop(0)),
                    trigger_threshold=int(values.pop(0)),
                    release_threshold=int(values.pop(0)),
                ),
                east=dict(
                    value=int(values.pop(0)),
                    trigger_threshold=int(values.pop(0)),
                    release_threshold=int(values.pop(0)),
                ),
                south=dict(
                    value=int(values.pop(0)),
                    trigger_threshold=int(values.pop(0)),
                    release_threshold=int(values.pop(0)),
                ),
                west=dict(
                    value=int(values.pop(0)),
                    trigger_threshold=int(values.pop(0)),
                    release_threshold=int(values.pop(0)),
                ),
            ),
            left=dict(
                north=dict(
                    value=int(values.pop(0)),
                    trigger_threshold=int(values.pop(0)),
                    release_threshold=int(values.pop(0)),
                ),
                east=dict(
                    value=int(values.pop(0)),
                    trigger_threshold=int(values.pop(0)),
                    release_threshold=int(values.pop(0)),
                ),
                south=dict(
                    value=int(values.pop(0)),
                    trigger_threshold=int(values.pop(0)),
                    release_threshold=int(values.pop(0)),
                ),
                west=dict(
                    value=int(values.pop(0)),
                    trigger_threshold=int(values.pop(0)),
                    release_threshold=int(values.pop(0)),
                ),
            ),
            right=dict(
                north=dict(
                    value=int(values.pop(0)),
                    trigger_threshold=int(values.pop(0)),
                    release_threshold=int(values.pop(0)),
                ),
                east=dict(
                    value=int(values.pop(0)),
                    trigger_threshold=int(values.pop(0)),
                    release_threshold=int(values.pop(0)),
                ),
                south=dict(
                    value=int(values.pop(0)),
                    trigger_threshold=int(values.pop(0)),
                    release_threshold=int(values.pop(0)),
                ),
                west=dict(
                    value=int(values.pop(0)),
                    trigger_threshold=int(values.pop(0)),
                    release_threshold=int(values.pop(0)),
                ),
            ),
        )
