"""Tests for the communicator class
"""

from unittest.mock import Mock
import pytest
from serial import Serial

from base.communicator import Communicator

from .stubs import PANEL_CONFIG_RESPONSE, SENSOR_VALUES_RESPONSE


class TestCommunicator:

    @pytest.fixture
    def setup(self):

        # Create mock Serial object
        self.mock_serial = Mock(spec=Serial)
        self.communicator = Communicator(ser=self.mock_serial)

    def test_get_version(self, setup):
        self.communicator.get_version()

        self.mock_serial.write.assert_called_once()
        self.mock_serial.readline.assert_called_once()

    def test_blink_led(self, setup):
        self.communicator.blink_led()

        self.mock_serial.write.assert_called_once()
        self.mock_serial.readline.assert_not_called()

    def test_get_panel_config(self, setup):
        self.mock_serial.readline.return_value = PANEL_CONFIG_RESPONSE

        self.communicator.get_panel_config()

        self.mock_serial.write.assert_called_once()
        self.mock_serial.readline.assert_called_once()

    def test_get_sensor_values(self, setup):
        self.mock_serial.readline.return_value = SENSOR_VALUES_RESPONSE

        self.communicator.get_sensor_values()

        self.mock_serial.write.assert_called_once()
        self.mock_serial.readline.assert_called_once()
