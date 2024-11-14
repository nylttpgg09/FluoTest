from PyQt5.QtWidgets import QApplication, QMainWindow, QWidget, QVBoxLayout, QPushButton, QStackedWidget
from PyQt5.uic import loadUi  # 用于加载 .ui 文件
import sys

class WelcomeWindow(QMainWindow):
    def __init__(self):
        super().__init__()
        loadUi("welcome.ui", self)  # 加载 welcome.ui
        self.main_button.clicked.connect(self.show_main_window)  # main.ui 按钮
        self.fluo_button.clicked.connect(self.show_fluo_window)  # FluoTest.ui 按钮


    def show_main_window(self):
        self.main_window = MainWindow()  # 创建主窗口对象
        self.main_window.show()
        self.close()  # 关闭当前的欢迎窗口

    def show_fluo_window(self):
        self.fluo_window = FluoTestWindow()  # 创建 FluoTest 窗口对象
        self.fluo_window.show()
        self.close()  # 关闭当前的欢迎窗口

class MainWindow(QMainWindow):
    def __init__(self):
        super().__init__()
        loadUi("main.ui", self)  # 加载 main.ui
        self.settingButton.clicked.connect(self.show_fluo_window)  # FluoTest.ui 按钮

    def show_fluo_window(self):
        self.fluo_window = FluoTestWindow()  # 创建 FluoTest 窗口对象
        self.fluo_window.show()
        self.close()  # 关闭当前

class FluoTestWindow(QMainWindow):
    def __init__(self):
        super().__init__()
        loadUi("FluoTest.ui", self)  # 加载 FluoTest.ui
        self.mainbutton.clicked.connect(self.show_main_window)  # FluoTest.ui 按钮
    def show_main_window(self):
        self.main_window = MainWindow()  # 创建主窗口对象
        self.main_window.show()
        self.close()  # 关闭当前的欢迎窗口

if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = WelcomeWindow()  # 创建欢迎窗口
    window.show()  # 显示欢迎窗口
    sys.exit(app.exec_())  # 启动应用
