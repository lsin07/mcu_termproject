import serial
from dashboard import Ui_DashBoard
from PyQt6.QtCore import QTimer
from PyQt6.QtWidgets import QApplication, QMainWindow

s_s32k = serial.Serial(port='COM8', timeout=0.15)

class Dashboard(QMainWindow, Ui_DashBoard):

    def __init__(self):
        super().__init__()
        self.setupUi(self)
        self.UART_timer = QTimer()
        self.UART_timer.setInterval(50)
        self.UART_timer.timeout.connect(self.UART_action)
        self.UART_timer.start()

    def UART_action(self):
        data = int.from_bytes(s_s32k.read(size=4))
        (blinker, gear, speed) = self.UART_input_parser(data)
        if data == 0:
            self.label_blinker_left.setText("No Signal")
            self.label_blinker_right.setText("No Signal")
            self.label_gear.setText("No Signal")
            self.lcd_speedometer.setProperty("value", 0)
        else:
            self.lcd_speedometer.setProperty("value", speed)

            if (blinker & 0b01) == 1:
                self.label_blinker_right.setText("On")
            else:
                self.label_blinker_right.setText("Off")
            if (blinker & 0b10) >> 1 == 1:
                self.label_blinker_left.setText("On")
            else:
                self.label_blinker_left.setText("Off")
                

            if gear == 1:
                self.label_gear.setText("P")
            elif gear == 2:
                self.label_gear.setText("R")
            elif gear == 3:
                self.label_gear.setText("D")
            else:
                self.label_gear.setText("ERROR")

    def UART_input_parser(self, d_in):
        blinker = (d_in & 0b110) >> 1
        gear = (d_in & 0b11000) >> 3
        speed = (d_in & 0b111111111100000) >> 5

        return (blinker, gear, speed)

if __name__ == '__main__':
    app = QApplication([])
    window = Dashboard()
    window.show()
    app.exec()