from PyQt5.QtWidgets import QApplication, QMainWindow, QProgressBar, QVBoxLayout, QWidget, QPushButton
from PyQt5.uic import loadUi
from PyQt5.QtSql import QSqlDatabase, QSqlQuery
from PyQt5.QtCore import QTimer
import sys
class Welcome1Window(QMainWindow):
    def __init__(self):
        super().__init__()
        loadUi("welcome1.ui", self)  # 加载 welcome1.ui

        # 获取进度条控件
        self.progress_bar = self.findChild(QProgressBar, "progressBar")  # 假设进度条名为 progressBar
        self.progress_bar.setValue(0)  # 初始值为 0

        # 设置定时器，用来更新进度条并跳转到主界面
        self.timer = QTimer(self)
        self.timer.timeout.connect(self.update_progress)
        self.timer.start(50)  # 每50毫秒更新一次进度条

        self.progress = 0  # 进度条的当前值

    def update_progress(self):
        self.progress += 1
        self.progress_bar.setValue(self.progress)

        # 如果进度条完成了100%，跳转到 main.ui
        if self.progress >= 100:
            self.timer.stop()  # 停止定时器
            self.open_welcome_window()  # 打开主界面

    def open_welcome_window(self):
        # 创建欢迎界面并显示
        self.welcome_window = WelcomeWindow()
        self.welcome_window.show()

        # 关闭欢迎界面
        self.close()

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
        self.settingButton.clicked.connect(self.show_fluo_window)
        self.historyButton.clicked.connect(self.show_history_window)
        self.itemButton.clicked.connect(self.show_item_window)
        self.testButton.clicked.connect(self.show_test_window)

    def show_fluo_window(self):
        self.fluo_window = FluoTestWindow()  # 创建 FluoTest 窗口对象
        self.fluo_window.show()
        self.close()  # 关闭当前

    def show_history_window(self):
        self.historys_window = HistorysWindow()
        self.historys_window.show()
        self.close()  # 关闭当前

    def show_item_window(self):
        self.item_window = QuerylineWindow()
        self.item_window.show()
        self.close()  # 关闭当前

    def show_test_window(self):
        self.test_window = ChannelWindow()
        self.test_window.show()
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

class HistorysWindow(QMainWindow):
    def __init__(self):
        super().__init__()
        loadUi("historys.ui", self)  # 加载 ui
        self.mainButton.clicked.connect(self.show_main_window)
    def show_main_window(self):
        self.main_window = MainWindow()  # 创建主窗口对象
        self.main_window.show()
        self.close()  # 关闭当前的欢迎窗口



class QuerylineWindow(QMainWindow):
    def __init__(self):
        super().__init__()
        loadUi("queryline.ui", self)  # 加载 ui


class ChannelWindow(QMainWindow):
    def __init__(self):
        super().__init__()
        loadUi("channel.ui", self)  # 加载 ui
        self.channelsetting.clicked.connect(self.show_setting_window)

    def show_setting_window(self):
        self.setting_window = ChannelsettingWindow()  # 创建主窗口对象
        self.setting_window.show()
        self.close()  # 关闭当前的欢迎窗口

class ChannelsettingWindow(QMainWindow):
    def __init__(self):
        super().__init__()
        loadUi("channelsetting.ui", self)  # 加载 ui
        self.controlButton.clicked.connect(self.show_control_window)
    def show_control_window(self):
        self.control_window = ChannelWindow()  # 创建主窗口对象
        self.control_window.show()
        self.close()  # 关闭当前的欢迎窗口

if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = Welcome1Window()  # 创建欢迎窗口
    window.show()  # 显示欢迎窗口
    sys.exit(app.exec_())  # 启动应用
