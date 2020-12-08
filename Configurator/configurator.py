"""GUI to configure the pad."""

import math
import sys
import time

from PyQt5.QtCore import QTimer
from PyQt5.QtWidgets import QApplication, QDialog
import numpy as np
import pyqtgraph as pg
import qdarkstyle

from gui.base import Ui_Dialog


class Dialog(QDialog, Ui_Dialog):
    """Implementation of the main dialog.
    """

    def __init__(self):
        super(Dialog, self).__init__()

        # Setup the user interface from Designer.
        self.setupUi(self)

        # Setup plots
        self.data_up = np.zeros(250)
        self.data_down = np.zeros(250)
        self.data_left = np.zeros(250)
        self.data_right = np.zeros(250)
        self.curve_up = self.plot_up.plot(
            self.data_up, pen=pg.mkPen('r', width=2))
        self.curve_down = self.plot_down.plot(
            self.data_down, pen=pg.mkPen('g', width=2))
        self.curve_left = self.plot_left.plot(
            self.data_left, pen=pg.mkPen('w', width=2))
        self.curve_right = self.plot_right.plot(
            self.data_right, pen=pg.mkPen('y', width=2))
        self.x = 0

        for plot in [self.plot_up, self.plot_down, self.plot_left, self.plot_right]:
            plot.showAxis('bottom', show=False)
            plot.getAxis('left').showLabel(False)

        self.time = time.time_ns()

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

        v = math.sin((3.14159/90) * self.x)

        # Update data for curves
        self.data_up = np.roll(self.data_up, -1)
        self.data_up[-1] = v
        self.data_down = np.roll(self.data_down, -1)
        self.data_down[-1] = v
        self.data_left = np.roll(self.data_left, -1)
        self.data_left[-1] = v
        self.data_right = np.roll(self.data_right, -1)
        self.data_right[-1] = v

        # Update curves
        self.curve_up.setData(self.data_up)
        self.curve_down.setData(self.data_down)
        self.curve_left.setData(self.data_left)
        self.curve_right.setData(self.data_right)

        self.x += 1


def main():
    """Entrypoint"""

    app = QApplication(sys.argv)
    app.setStyleSheet(qdarkstyle.load_stylesheet_pyqt5())
    gui = Dialog()
    gui.show()
    sys.exit(app.exec_())


if __name__ == '__main__':
    main()
