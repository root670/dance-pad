"""Communicate with the board firmware.
"""

from enum import Enum
from typing import Mapping, Tuple, Union
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


class Color:

    @staticmethod
    def from_int(value: int) -> Tuple[int, int, int]:
        return (value & 0xFF, (value >> 8) & 0xFF, (value >> 16) & 0xFF)

    @staticmethod
    def to_int(color: Tuple[int, int, int]) -> int:

        if len(color) != 3:
            raise ValueError("`color` must be a 3-tuple")

        for component in range(len(color)):
            value = color[component]
            if value < 0 or value > 255:
                raise ValueError(
            '{component} component of color must be in range [0, 255].'.format(
                component='RGB'[component]
            ))

        return color[0] | (color[1] << 8) | (color[2] << 16)


class Communicator:
    """Communicate with the board firmware.
    """

    COMMAND_VERSION = 'version'
    COMMAND_BLINK = 'blink'
    COMMAND_GETCONFIG = 'config'
    COMMAND_SETCONFIG = 'set'
    COMMAND_PERSIST = 'persist'
    COMMAND_VALUES = 'v'
    COMMAND_CALIBRATE = 'calibrate'

    CONFIG_TYPE_STRING = 'str'
    CONFIG_TYPE_U16 = 'u16'
    CONFIG_TYPE_U32 = 'u32'

    RESPONSE_SUCCESS = '!'
    RESPONSE_FAILURE = '?'

    DIRECTIONS = {'up', 'down', 'left', 'right'}

    def __init__(
        self,
        ser: Union[serial.Serial, str],
    ) -> None:
        """Communicate with the firmware using a serial interface.

        Args:
            ser: Serial object
        """
        if isinstance(ser, str):
            ser = serial.Serial(ser)

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

    def __set_config(self, value_type: str, key: str, value) -> None:
        """Set a configuration item.

        Args:
            value_type: One of {`str`, `u16`, `u32`}.
            key: Name of configuration item.
            value: The value to set the configuration item to.
        """
        self.__send_command(self.COMMAND_SETCONFIG)
        self.__send_line(f'{value_type} {key}={value}')
        response = self.__get_line()

        if not response in {
            Communicator.RESPONSE_SUCCESS,
            Communicator.RESPONSE_FAILURE
        }:
            raise ValueError(f'Unexpected response: {response}')

        if response != self.RESPONSE_SUCCESS:
            raise ValueError(f'Failed to set config {key}[{value_type}]={value}')

    def __set_config_u16(self, key: str, value) -> None:
        """Set a configuration item for an unsigned 16-bit integer.

        Args:
            key: Name of the configuration item.
            value: The value to set the configuration item to.
        """
        self.__set_config(self.CONFIG_TYPE_U16, key, value)

    def __set_config_u32(self, key: str, value) -> None:
        """Set a configuration item for an unsigned 32-bit integer.

        Args:
            key: Name of the configuration item.
            value: The value to set the configuration item to.
        """
        self.__set_config(self.CONFIG_TYPE_U32, key, value)

    def __set_config_str(self, key: str, value) -> None:
        """Set a configuration item for a string.

        Args:
            key: Name of the configuration item.
            value: The value to set the configuration item to.
        """
        self.__set_config(self.CONFIG_TYPE_STRING, key, value)

    def get_version(self) -> str:
        """Get firmware version string.
        """
        self.__send_command(self.COMMAND_VERSION)
        return self.__get_line()

    def blink_led(self) -> None:
        """Blink the on-board LED on the Teensy board.
        """
        self.__send_command(self.COMMAND_BLINK)

    def calibrate(self) -> None:
        """Force calibration of the sensors.
        """
        self.__send_command(self.COMMAND_CALIBRATE)

    def set_thresholds(self, pin: int, trigger: int, release:int) -> None:
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

        self.__set_config_u16(key_name + 'trigger', trigger)
        self.__set_config_u16(key_name + 'release', release)

    def get_config(self) -> Mapping[str, Enum]:
        """Get configuration.

        Returns:
            Dictionary of configuration items.
        """
        self.__send_command(self.COMMAND_GETCONFIG)
        response = self.__get_line()
        split = response.split(',')

        key_value_entries = dict(entry.split('=') for entry in split[20:])

        return dict(
            up=dict(
                orientation=PanelOrientation.from_degrees(int(split.pop(0))),
                color=Color.from_int(int(key_value_entries['color_up'])),
                north_pin=int(split.pop(0)),
                east_pin=int(split.pop(0)),
                south_pin=int(split.pop(0)),
                west_pin=int(split.pop(0)),
            ),
            down=dict(
                orientation=PanelOrientation.from_degrees(int(split.pop(0))),
                color=Color.from_int(int(key_value_entries['color_down'])),
                north_pin=int(split.pop(0)),
                east_pin=int(split.pop(0)),
                south_pin=int(split.pop(0)),
                west_pin=int(split.pop(0)),
            ),
            left=dict(
                orientation=PanelOrientation.from_degrees(int(split.pop(0))),
                color=Color.from_int(int(key_value_entries['color_left'])),
                north_pin=int(split.pop(0)),
                east_pin=int(split.pop(0)),
                south_pin=int(split.pop(0)),
                west_pin=int(split.pop(0)),
            ),
            right=dict(
                orientation=PanelOrientation.from_degrees(int(split.pop(0))),
                color=Color.from_int(int(key_value_entries['color_right'])),
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

    def set_color(self, panel, r, g, b) -> None:
        if panel not in self.DIRECTIONS:
            raise ValueError('{panel} must be one of: {directions}'.format(
                panel=panel,
                directions=', '.join(self.DIRECTIONS)
        ))

        rgb = Color.to_int((r, g, b))
        self.__set_config_u32(f'color_{panel}', rgb)

    def set_arrow_lights(self, enabled) -> None:
        """Turn all the arrow lights on or off
        """
        if enabled:
            self.__send_line('@@@O@@@@@@@@@')
        else:
            self.__send_line('@@@@@@@@@@@@@')