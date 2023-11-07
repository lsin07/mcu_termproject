from PyQt6. QtCore import QSize, Qt
from PyQt6.QtWidgets import QApplication, QPushButton, QMainWindow

class MainWindow(QMainWindow):
    def __init__(self):
        super().__init__()

        self.setWindowTitle("Dashboard")

        button = QPushButton("Press Me!")
        button.setCheckable(True)
        button.clicked.connect(self.the_button_was_clicked)
        button.clicked.connect(self.the_button_was_toggled)

        self.setCentralWidget(button)

    def the_button_was_clicked(self):
        print("clicked!")
    
    def the_button_was_toggled(self, checked):
        print("checked?", checked)

app = QApplication([])

window = MainWindow()
window.show()

app.exec()