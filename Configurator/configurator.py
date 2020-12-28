"""GUI to configure the pad."""

import math
import sys
import time
import os

from PyQt5 import uic
from PyQt5.QtCore import QTimer
from PyQt5.QtWidgets import QApplication, QDialog, QTableWidgetItem
import numpy as np
import pyqtgraph as pg
import qdarkstyle
from serial.tools import list_ports
from serial import Serial

from base.communicator import Communicator


class Dialog(QDialog):
    """Implementation of the main dialog.
    """

    UI_FILE = os.path.join('resources', 'gui.ui')

    def __init__(self):
        super(Dialog, self).__init__()

        # Setup the user interface from Designer.
        uic.loadUi(self.UI_FILE, self)

        self.comm = None

        self.serial_ports = list_ports.comports()
        self.comboBoxDevices.activated.connect(self.on_device_changed)
        self.comboBoxDevices.addItem('Devices')
        for index, port in enumerate(self.serial_ports):
            self.comboBoxDevices.addItem(f'{index}: {port.device}')

        # Setup plots
        self.data_sensors = [np.zeros(250, dtype=np.int16)] * 16

        self.curves_up = [
            self.plot_up.plot(self.data_sensors[0], pen=pg.mkPen('r', width=3)),
            self.plot_up.plot(self.data_sensors[1], pen=pg.mkPen('g', width=3)),
            self.plot_up.plot(self.data_sensors[2], pen=pg.mkPen('w', width=3)),
            self.plot_up.plot(self.data_sensors[3], pen=pg.mkPen('y', width=3)),
        ]
        self.curves_down = [
            self.plot_down.plot(self.data_sensors[4], pen=pg.mkPen('r', width=3)),
            self.plot_down.plot(self.data_sensors[5], pen=pg.mkPen('g', width=3)),
            self.plot_down.plot(self.data_sensors[6], pen=pg.mkPen('w', width=3)),
            self.plot_down.plot(self.data_sensors[7], pen=pg.mkPen('y', width=3)),
        ]
        self.curves_left = [
            self.plot_left.plot(self.data_sensors[8], pen=pg.mkPen('r', width=3)),
            self.plot_left.plot(self.data_sensors[9], pen=pg.mkPen('g', width=3)),
            self.plot_left.plot(self.data_sensors[10], pen=pg.mkPen('w', width=3)),
            self.plot_left.plot(self.data_sensors[11], pen=pg.mkPen('y', width=3)),
        ]
        self.curves_right = [
            self.plot_right.plot(self.data_sensors[12], pen=pg.mkPen('r', width=3)),
            self.plot_right.plot(self.data_sensors[13], pen=pg.mkPen('g', width=3)),
            self.plot_right.plot(self.data_sensors[14], pen=pg.mkPen('w', width=3)),
            self.plot_right.plot(self.data_sensors[15], pen=pg.mkPen('y', width=3)),
        ]

        self.x = 0

        for plot in [
            self.plot_up,
            self.plot_down,
            self.plot_left,
            self.plot_right,
        ]:
            plot.showAxis('bottom', show=False)
            plot.getAxis('left').showLabel(False)

        self.time = time.time_ns()

        # Setup table
        for row in range(16):
            self.tableThresholds.setItem(row, 0, QTableWidgetItem(str(row)))
            self.tableThresholds.setItem(row, 0, QTableWidgetItem('-'))
            self.tableThresholds.setItem(row, 1, QTableWidgetItem('500'))
            self.tableThresholds.setItem(row, 2, QTableWidgetItem('400'))

        # Call the update function periodically
        timer = QTimer(self)
        timer.timeout.connect(self.update_plots)
        timer.start(0)

    def update_plots(self):
        """Plot the current sensor values."""

        if self.comm is None:
            return

        # Compute FPS
        x = time.time_ns()
        print(1000 / ((x - self.time)/1e6))
        self.time = x

        # Update data for curves
        for idx in range(len(self.data_sensors)):
            self.data_sensors[idx] = np.roll(self.data_sensors[idx], -1)

        values = self.comm.get_sensor_values()
        self.data_sensors[0][-1] = values['up']['north']['value']
        self.data_sensors[1][-1] = values['up']['east']['value']
        self.data_sensors[2][-1] = values['up']['south']['value']
        self.data_sensors[3][-1] = values['up']['west']['value']
        self.data_sensors[4][-1] = values['down']['north']['value']
        self.data_sensors[5][-1] = values['down']['east']['value']
        self.data_sensors[6][-1] = values['down']['south']['value']
        self.data_sensors[7][-1] = values['down']['west']['value']
        self.data_sensors[8][-1] = values['left']['north']['value']
        self.data_sensors[9][-1] = values['left']['east']['value']
        self.data_sensors[10][-1] = values['left']['south']['value']
        self.data_sensors[11][-1] = values['left']['west']['value']
        self.data_sensors[12][-1] = values['right']['north']['value']
        self.data_sensors[13][-1] = values['right']['east']['value']
        self.data_sensors[14][-1] = values['right']['south']['value']
        self.data_sensors[15][-1] = values['right']['west']['value']

        # Update curves
        y = 0
        for direction in [self.curves_up, self.curves_down, self.curves_left, self.curves_right]:
            for sensor in direction:
                self.tableThresholds.item(y, 1).setText(str(self.data_sensors[y][-1]))
                sensor.setData(self.data_sensors[y])
                y += 1

        self.x += 1

    def on_device_changed(self, index: int):

        device = self.serial_ports[index - 1].device
        try:
            serial = Serial(device)
        except Exception as e:
            self.labelDeviceInfo.setText(str(e))
            return

        self.comm = Communicator(
            ser=serial
        )

        device_info = self.comm.get_version()
        config = self.comm.get_config()

        self.tableThresholds.item(0,0).setText(str(config['up']['north_pin']))
        self.tableThresholds.item(1,0).setText(str(config['up']['east_pin']))
        self.tableThresholds.item(2,0).setText(str(config['up']['south_pin']))
        self.tableThresholds.item(3,0).setText(str(config['up']['west_pin']))
        self.tableThresholds.item(4,0).setText(str(config['down']['north_pin']))
        self.tableThresholds.item(5,0).setText(str(config['down']['east_pin']))
        self.tableThresholds.item(6,0).setText(str(config['down']['south_pin']))
        self.tableThresholds.item(7,0).setText(str(config['down']['west_pin']))
        self.tableThresholds.item(8,0).setText(str(config['left']['north_pin']))
        self.tableThresholds.item(9,0).setText(str(config['left']['east_pin']))
        self.tableThresholds.item(10,0).setText(str(config['left']['south_pin']))
        self.tableThresholds.item(11,0).setText(str(config['left']['west_pin']))
        self.tableThresholds.item(12,0).setText(str(config['right']['north_pin']))
        self.tableThresholds.item(13,0).setText(str(config['right']['east_pin']))
        self.tableThresholds.item(14,0).setText(str(config['right']['south_pin']))
        self.tableThresholds.item(15,0).setText(str(config['right']['west_pin']))

        self.labelDeviceInfo.setText(f'Connected to {device}.\n\n{device_info}')


def main():
    """Entrypoint"""

    app = QApplication(sys.argv)
    app.setStyleSheet(qdarkstyle.load_stylesheet_pyqt5())
    gui = Dialog()
    gui.show()
    sys.exit(app.exec_())


if __name__ == '__main__':
    main()
