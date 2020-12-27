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
            self.tableThresholds.setItem(row, 0, QTableWidgetItem('A0'))
            self.tableThresholds.setItem(row, 1, QTableWidgetItem('500'))
            self.tableThresholds.setItem(row, 2, QTableWidgetItem('400'))

        # Call the update function periodically
        timer = QTimer(self)
        timer.timeout.connect(self.update_plots)
        timer.start(0)

    def update_plots(self):
        """Plot the current sensor values."""

        # Compute FPS
        x = time.time_ns()
        print(1000 / ((x - self.time)/1e6))
        self.time = x

        # Update data for curves
        for idx in range(len(self.data_sensors)):
            self.data_sensors[idx] = np.roll(self.data_sensors[idx], -1)
            self.data_sensors[idx][-1] = max(int(math.sin((3.14159/(90 * (idx + 1))) * self.x) * 1023), 0)

        # Update curves
        y = 0
        for direction in [self.curves_up, self.curves_down, self.curves_left, self.curves_right]:
            for sensor in direction:
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
