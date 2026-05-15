import sys
import os
from PySide6 import QtWidgets, QtUiTools, QtCore
from PySide6.QtWidgets import QApplication
from PySide6.QtGui import QGuiApplication
import pyqtgraph as pg
import numpy as np

sys.path.append(os.path.abspath("build"))

import solver

from GUI_shema import Ui_MainWindow

class KCR(QtWidgets.QMainWindow):
    def __init__(self):
        super().__init__()

        self.ui = Ui_MainWindow()
        self.ui.setupUi(self)

        self.setCentralWidget(self.ui.centralwidget)
        screen = QGuiApplication.primaryScreen()
        geometry = screen.availableGeometry()
        screen_width = geometry.width()
        window_height = 600
        self.setGeometry(0,0,screen_width,window_height)

        solver.Solution(80, 160, 0.0, 1.0, 0.0, 2.0, 1e-5, 50000)

   # def change_table(self):

if __name__ == "__main__":
    app = QtWidgets.QApplication(sys.argv)
    window = KCR()
    window.show()
    sys.exit(app.exec())