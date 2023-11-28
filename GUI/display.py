from serial import Serial, serialutil
from dashboard import Ui_DashBoard
from PyQt6.QtCore import QTimer, Qt
from PyQt6.QtWidgets import QApplication, QMainWindow
from PyQt6.QtGui import QPixmap, QTransform

# 여기에 포트 번호 입력
port = 'COM13'

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
        self.label_speedometer_bg.setPixmap(QPixmap(".\\resources/speedometer_bg.png"))

    def UART_action(self):
        try:
            (lamp, blinker, gear, speed, prox, errorNo) = self.UART_input_parser(int.from_bytes(s_s32k.read(size=4)))
        except serialutil.SerialException:
            (lamp, blinker, gear, speed, prox, errorNo) = (0, 0, 0, 0, 0, 4)

        self.transform_speedometer_rotate.reset()

        if errorNo == 1: # not connected
            self.label_gear.setText("E1")
            self.label_speedometer_digits.setText("NO SIGNAL")
        elif errorNo == 2: # invalid gear input
            self.label_gear.setText("E2")
            self.label_speedometer_digits.setText("GEAR")
        elif errorNo == 4: # connection lost
            self.label_gear.setText("E4")
            self.label_speedometer_digits.setText("CONN LOST")
        else:
            # lamp
            if lamp == 0:
                self.label_poslight.setPixmap(QPixmap(".\\resources/poslight_off.png"))
                self.label_light.setPixmap(QPixmap(".\\resources/light_off.png"))
            elif lamp == 1:
                self.label_poslight.setPixmap(QPixmap(".\\resources/poslight_on.png"))
                self.label_light.setPixmap(QPixmap(".\\resources/light_off.png"))
            elif lamp == 2:
                self.label_poslight.setPixmap(QPixmap(".\\resources/poslight_on.png"))
                self.label_light.setPixmap(QPixmap(".\\resources/light_on.png"))

            # speedometer
            self.label_speedometer_digits.setText(str("{:.2f} m/min").format(round(speed * 0.00739, 2))) # 0.00739 = 7.56 / 1023
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
            
            # proximity warning
            if prox == 1:
                self.widget_prox_label_text.setText("전방 추돌 주의")
                self.widget_prox_label_icon.setPixmap(QPixmap(".\\resources/prox.png"))
                self.widget_prox.show()
            elif prox == 2:
                self.widget_prox_label_text.setText("전방 추돌 경고")
                self.widget_prox_label_icon.setPixmap(QPixmap(".\\resources/warning.png"))
                self.widget_prox.show()
            else:
                self.widget_prox.hide()

    def UART_input_parser(self, d_in):

        lamp = (d_in & 0b11)                            # bit 0-1: light
        blinker = (d_in & 0b1100) >> 2                  # bit 2-3: blinker
        gear = (d_in & 0b110000) >> 4                   # bit 4-5: gear
        speed = (d_in & 0xFFC0) >> 6                    # bit 6-15: speed
        prox = (d_in & 0x30000) >> 16

        if d_in == 0: errorNo = 1                       # errno 1: No signal
        elif gear == 0: errorNo = 2                     # errno 2: invalid gear input
        else: errorNo = 0                               # errno 0: No error

        return (lamp, blinker, gear, speed, prox, errorNo)
    
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