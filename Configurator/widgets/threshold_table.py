from PyQt6.QtWidgets import QTableWidget


class ThresholdTable(QTableWidget):
    def __init__(self, parent=None):
        super(ThresholdTable, self).__init__(parent)

    def keyPressEvent(self, event):
        key = event.key()

        if key == 16777220:
            if self.currentColumn() in {0, 1}:
                return  # Ignore the Pin and Value column

            cur = self.currentItem()
            font = cur.font()
            font.setBold(True)
            cur.setFont(font)
        else:
            super(ThresholdTable, self).keyPressEvent(event)
