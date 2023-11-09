from serial import Serial, serialutil
from dashboard import Ui_DashBoard
from PyQt6.QtCore import QTimer, Qt
from PyQt6.QtWidgets import QApplication, QMainWindow
from PyQt6.QtGui import QPixmap, QTransform

port = 'COM7'
try:
    s_s32k = Serial(port=port, timeout=0.15)
except serialutil.SerialException:
    print("Connection to port " + port + " failed.")
    exit(1)

class Dashboard(QMainWindow, Ui_DashBoard):

    def __init__(self):
        super().__init__()
        self.setupUi(self)
        
        # UART read Timer
        self.UART_timer = QTimer()
        self.UART_timer.setInterval(50)
        self.UART_timer.timeout.connect(self.UART_action)
        self.UART_timer.start()

        # Pixmap & Transform
        self.transform_speedometer_rotate = QTransform()
        self.pixmap_speedometer = QPixmap(".\\resources/speedometer.png")
        self.label_speedometer_analog.setPixmap(self.pixmap_speedometer)

    def UART_action(self):
        try:
            (blinker, gear, speed, errorNo) = self.UART_input_parser(int.from_bytes(s_s32k.read(size=4)))
        except serialutil.SerialException:
            (blinker, gear, speed, errorNo) = (0, 0, 0, 4)

        self.transform_speedometer_rotate.reset()

        if errorNo == 1: # not connected
            self.label_gear.setText("E1")
            self.label_speedometer_digits.setText("NO SIGNAL")
        elif errorNo == 2: # invalid gear input
            self.label_gear.setText("E2")
            self.label_speedometer_digits.setText("GEAR")
        # TODO: main.c에 패리티 비트 구현하기.
        # if errorNo == 3: # invalid parity
        #     self.label_gear.setText("E3")
        #     self.label_speedometer_digits.setText("PARITY")
        elif errorNo == 4: # connection lost
            self.label_gear.setText("E4")
            self.label_speedometer_digits.setText("CONN LOST")
        else:
            # speedometer
            self.label_speedometer_digits.setText(str(int(speed)) + " km/h")
            self.transform_speedometer_rotate.rotate(self.getSpeedometerDeg(int(speed)))
            self.label_speedometer_analog.setPixmap(self.pixmap_speedometer.transformed(self.transform_speedometer_rotate, Qt.TransformationMode.SmoothTransformation))

            # blinker
            if (blinker & 0b01) == 1:
                self.label_blinker_right.setPixmap(QPixmap(".\\resources/right_arrow_on.png"))
            else:
                self.label_blinker_right.setPixmap(QPixmap(".\\resources/right_arrow_off.png"))
            if (blinker & 0b10) >> 1 == 1:
                self.label_blinker_left.setPixmap(QPixmap(".\\resources/left_arrow_on.png"))
            else:
                self.label_blinker_left.setPixmap(QPixmap(".\\resources/left_arrow_off.png"))
                
            # gear
            if gear == 0b01:
                self.label_gear.setText("P")
            elif gear == 0b10:
                self.label_gear.setText("R")
            elif gear == 0b11:
                self.label_gear.setText("D")

    def UART_input_parser(self, d_in):

        blinker = (d_in & 0b110) >> 1                   # bit 1-2: blinker
        gear = (d_in & 0b11000) >> 3                    # bit 3-4: gear
        speed = (d_in & 0b111111111100000) >> 5         # bit 5-14: speed

        if d_in == 0: errorNo = 1                       # errno 1: No signal
        elif gear == 0: errorNo = 2                     # errno 2: invalid gear input
        elif self.parityCheck(d_in) != 0: errorNo = 3   # errno 3: invalid parity
        else: errorNo = 0                               # errno 0: No error

        return (blinker, gear, speed, errorNo)
    
    def parityCheck(self, x):
        res = 0
        while x:
            res ^= x & 1
            x >>= 1
        return res
    
    def getSpeedometerDeg(self, speed):
        # 회전 각도: 250 ~ 470
        min_deg = 250
        max_deg = 470
        max_speed = 1023
        return speed * (max_deg - min_deg) / max_speed + min_deg

if __name__ == '__main__':
    app = QApplication([])
    window = Dashboard()
    window.show()
    app.exec()