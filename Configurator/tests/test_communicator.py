"""Tests for the communicator class
"""

from unittest.mock import Mock
import pytest
from serial import Serial

from base.communicator import Communicator, PanelOrientation

from .stubs import GET_CONFIG_RESPONSE, SENSOR_VALUES_RESPONSE

class TestPanelConfiguration:

    def test_should_return_value_for_valid_degrees(self):
        for degrees in [0, 90, 180, 270]:
            ret = PanelOrientation.from_degrees(degrees)
            assert type(ret) == PanelOrientation

    def test_should_throw_exception_for_bad_degrees(self):
        with pytest.raises(Exception):
            PanelOrientation.from_degrees(-1)
            PanelOrientation.from_degrees()
            PanelOrientation.from_degrees('0')
            PanelOrientation.from_degrees(360)


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

    def test_get_config(self, setup):
        self.mock_serial.readline.return_value = GET_CONFIG_RESPONSE

        self.communicator.get_config()

        self.mock_serial.write.assert_called_once()
        self.mock_serial.readline.assert_called_once()

    def test_set_thresholds(self, setup):
        self.mock_serial.readline.return_value = Communicator.RESPONSE_SUCCESS.encode('ascii')

        self.communicator.set_thresholds(1, 500, 400)

        assert self.mock_serial.write.call_count == 4
        assert self.mock_serial.readline.call_count == 2

    def test_set_thresholds_with_wrong_values(self, setup):
        with pytest.raises(ValueError):
            self.communicator.set_thresholds(1, 10000, 100)
            self.communicator.set_thresholds(1, 10000, 5000)
            self.communicator.set_thresholds(1, 0, -1)
            self.communicator.set_thresholds(1, -1, -2)
            self.communicator.set_thresholds(1, 600, 601)

    def test_get_sensor_values(self, setup):
        self.mock_serial.readline.return_value = SENSOR_VALUES_RESPONSE

        self.communicator.get_sensor_values()

        self.mock_serial.write.assert_called_once()
        self.mock_serial.readline.assert_called_once()
