"""GUI to configure the pad."""

import math
import sys
import time
import os
import re

from PyQt5 import uic
from PyQt5.QtCore import Qt, QTimer
from PyQt5.QtWidgets import QApplication, QDialog, QTableWidgetItem, QColorDialog
from PyQt5.QtGui import QColor
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

        self.pushButton_reset.clicked.connect(self.on_reset_clicked)
        self.pushButton_save.clicked.connect(self.on_save_clicked)

        # Setup lighting controls
        self.pushButton_colorUp.clicked.connect(self.on_up_color_clicked)
        self.pushButton_colorDown.clicked.connect(self.on_down_color_clicked)
        self.pushButton_colorLeft.clicked.connect(self.on_left_color_clicked)
        self.pushButton_colorRight.clicked.connect(self.on_right_color_clicked)

        self.horizontalSlider_brightness.valueChanged.connect(self.on_brightness_changed)
        self.checkBox_displayLights.toggled.connect(self.on_display_lights_toggled)
        self.checkBox_autoLights.toggled.connect(self.on_auto_lights_toggled)

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
            plot.setYRange(0, 1023, padding=0)

        self.time = time.time_ns()

        # Setup table
        for row in range(16):
            pin_item = QTableWidgetItem(str(row))
            pin_item.setFlags(pin_item.flags() & ~Qt.ItemIsEnabled)
            self.tableThresholds.setItem(row, 0, pin_item)
            value_item = QTableWidgetItem('-')
            value_item.setFlags(value_item.flags() & ~Qt.ItemIsEditable)
            self.tableThresholds.setItem(row, 1,value_item)
            self.tableThresholds.setItem(row, 2, QTableWidgetItem('-'))
            self.tableThresholds.setItem(row, 3, QTableWidgetItem('-'))

        # Set all
        self.lineEdit_trigger.setText('150')
        self.lineEdit_release.setText('110')
        self.pushButton_setAll.clicked.connect(self.on_set_all_clicked)

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
        # print(1000 / ((x - self.time)/1e6))
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

        self.labelDeviceInfo.setText(self.comm.get_version())

        config = self.comm.get_config()

        # Update table
        idx = 0
        for panel in ['up', 'down', 'left', 'right']:
            for direction in ['north', 'east', 'south', 'west']:
                self.tableThresholds.item(idx,0).setText(str(config[panel][f'{direction}_pin']))
                self.tableThresholds.item(idx,2).setText(config[panel][f'{direction}_trigger'])
                self.tableThresholds.item(idx,3).setText(config[panel][f'{direction}_release'])
                idx += 1

        self.tableThresholds.itemChanged.connect(self.on_table_item_changed)

        # Lights
        self.pushButton_colorUp.setStyleSheet(
            'background-color: rgb({},{},{})'.format(*config['up']['color'])
        )
        self.pushButton_colorDown.setStyleSheet(
            'background-color: rgb({},{},{})'.format(*config['down']['color'])
        )
        self.pushButton_colorLeft.setStyleSheet(
            'background-color: rgb({},{},{})'.format(*config['left']['color'])
        )
        self.pushButton_colorRight.setStyleSheet(
            'background-color: rgb({},{},{})'.format(*config['right']['color'])
        )

        # Lights
        self.labelBrightness.setText(str(config['brightness']))
        self.horizontalSlider_brightness.setValue(config['brightness'])
        self.checkBox_autoLights.setCheckState(2 if config['auto_lights'] else 0)

        self.config = config

    def on_reset_clicked(self):
        self.comm.calibrate()

    def on_save_clicked(self):
        self.comm.persist()

    @staticmethod
    def __get_color_from_stylesheet(stylesheet):
        """Get a 3-tuple of the RGB value from the `background-color` property
        in `stylesheet`.
        """
        pattern = re.compile(r'background-color: rgb\(([0-9]+),\s*([0-9]+),\s*([0-9]+)\)')
        return tuple(map(int, pattern.search(stylesheet).groups()))

    def __on_color_clicked(self, widget, panel):
        current_color = self.__get_color_from_stylesheet(widget.styleSheet())
        color = QColorDialog.getColor(QColor.fromRgb(*current_color))
        if not color.isValid():
            return # Closed the color dialog

        color_rgb = (color.red(), color.green(), color.blue())
        widget.setStyleSheet(
            'background-color: rgb({},{},{})'.format(*color_rgb)
        )

        self.comm.set_color(panel, *color_rgb)

    def on_up_color_clicked(self):
        self.__on_color_clicked(self.pushButton_colorUp, 'up')
    
    def on_down_color_clicked(self):
        self.__on_color_clicked(self.pushButton_colorDown, 'down')

    def on_left_color_clicked(self):
        self.__on_color_clicked(self.pushButton_colorLeft, 'left')

    def on_right_color_clicked(self):
        self.__on_color_clicked(self.pushButton_colorRight, 'right')

    def on_display_lights_toggled(self, enabled):
        self.comm.set_arrow_lights(enabled)

    def on_auto_lights_toggled(self, enabled):
        self.comm.set_auto_lights(enabled)

    def on_brightness_changed(self, brightness):
        self.labelBrightness.setText(str(brightness))
        self.comm.set_brightness(brightness)

    def on_set_all_clicked(self):
        trigger = int(self.lineEdit_trigger.text())
        release = int(self.lineEdit_release.text())
        for panel in ['up', 'down', 'left', 'right']:
            for direction in ['north', 'east', 'south', 'west']:
                self.comm.set_thresholds(self.config[panel][f'{direction}_pin'], trigger, release)
        self.comm.calibrate()
        for row in range(16):
            self.tableThresholds.item(row,2).setText(str(trigger))
            self.tableThresholds.item(row,3).setText(str(release))

    def on_table_item_changed(self, item):
        if item.column() == 1:
            return  # Ignore Value column
        row = int(item.row())
        col = int(item.column())
        pin = int(self.tableThresholds.item(row, 0).text())

        trigger = int(item.text() if col == 2 else self.tableThresholds.item(row, 2).text())
        release = int(item.text() if col == 3 else self.tableThresholds.item(row, 3).text())
        print(f'trigger={trigger} release={release}')

        self.comm.set_thresholds(pin, trigger, release)


def main():
    """Entrypoint"""

    app = QApplication(sys.argv)
    app.setStyleSheet(qdarkstyle.load_stylesheet_pyqt5())
    gui = Dialog()
    gui.show()
    sys.exit(app.exec_())


if __name__ == '__main__':
    main()
