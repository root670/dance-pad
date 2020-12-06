import sys
from PyQt5.QtWidgets import QApplication, QWidget


class GUI(QWidget):

    def __init__(self):
        super().__init__()

        self.resize(250, 150)
        self.move(300, 300)
        self.setWindowTitle('Dance Pad')
        self.show()


def main():

    app = QApplication(sys.argv)
    gui = GUI()
    sys.exit(app.exec_())


if __name__ == '__main__':
    main()