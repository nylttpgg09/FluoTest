from PyQt5.QtWidgets import QProgressBar
from PyQt5.uic import loadUi
from PyQt5.QtCore import QTimer
import sys
import numpy as np
from scipy.signal import find_peaks
import matplotlib.pyplot as plt
from matplotlib.backends.backend_qt5agg import FigureCanvasQTAgg as FigureCanvas
from PyQt5.QtWidgets import QApplication, QMainWindow, QVBoxLayout, QWidget, QLabel, QPushButton
# 读取数据
data_values = []

def load_data():
    global data_values
    data_values = []
    # 打开文件并读取
    with open('data.txt', 'r') as file:
        hex_data = file.read().split()
        hex_data = hex_data[6:]

        for i in range(0, len(hex_data), 2):
            hex_pair = hex_data[i:i + 2]
            if len(hex_pair) == 2:
                low_byte = int(hex_pair[0], 16)
                high_byte = int(hex_pair[1], 16)
                value = low_byte + (high_byte << 8)
                data_values.append(value)

    data_values = np.array(data_values)


def calculate_peak_area_base_line(data):
    # 通过 find_peaks 查找波峰，并调整参数
    peaks, properties = find_peaks(data, prominence=100, width=1)
    peak_areas = []

    for peak in peaks:
        left = peak
        right = peak

        # 动态调整左右扩展范围
        while left > 0 and data[left] > data[left - 1]:
            left -= 10  # 根据需要调整步长
        while right < len(data) - 1 and data[right] > data[right + 1]:
            right += 10

        # 扩展检测范围
        left_extended = max(0, left - 100)
        right_extended = min(len(data) - 1, right + 100)

        # 使用移动平均来平滑基线
        baseline_left = np.mean(data[left_extended:peak + 1])
        baseline_right = np.mean(data[peak:right_extended + 1])
        baseline = min(baseline_left, baseline_right)

        # 计算波峰面积
        peak_area = np.trapz(data[left:right + 1] - baseline, dx=1)
        peak_areas.append(peak_area)

    return peaks, peak_areas


# 选择x轴在500-700和700-900之间的最大两个波峰进行比较
def get_max_peak_in_range(peaks, peak_areas, x_range):
    # 筛选在指定范围内的波峰
    valid_peaks = [(i, peak_areas[i]) for i in range(len(peaks)) if x_range[0] <= peaks[i] <= x_range[1]]

    if len(valid_peaks) == 0:
        return None, None  # 如果范围内没有波峰

    # 找到最大面积的波峰
    max_peak = max(valid_peaks, key=lambda x: x[1])

    return max_peak


# 选择500-700区间和700-900区间的最大波峰
def calculate_area_ratio(peaks, peak_areas):
    max_peak_500_700 = get_max_peak_in_range(peaks, peak_areas, (500, 700))
    max_peak_700_900 = get_max_peak_in_range(peaks, peak_areas, (700, 900))

    if max_peak_500_700 and max_peak_700_900:
        # 直接获取波峰的索引
        peak_500_700_index = max_peak_500_700[0]
        peak_700_900_index = max_peak_700_900[0]

        # 输出两个区间的最大波峰信息
        print(f"Max peak in range 500-700: Peak {peak_500_700_index + 1} with area: {max_peak_500_700[1]}")
        print(f"Max peak in range 700-900: Peak {peak_700_900_index + 1} with area: {max_peak_700_900[1]}")

        # 计算这两个最大波峰的面积比值
        peak_area_ratio = max_peak_500_700[1] / max_peak_700_900[1]
        print(
            f"The ratio of the max peak area in 500-700 range to the max peak area in 700-900 range is: {peak_area_ratio}")

        return peak_area_ratio
    else:
        print("One or both of the ranges do not contain any peaks.")
        return None


class PlotCanvas(FigureCanvas):
    def __init__(self, parent=None):
        # 创建一个图形对象
        self.fig, self.ax = plt.subplots(figsize=(4.7, 2.8))

        super().__init__(self.fig)
        self.setParent(parent)

    def plot(self, data, peaks):
        # 清除上一次生成的图像
        self.ax.clear()

        # 绘制原始曲线
        self.ax.plot(range(len(data)), data, label='Original Data')

        # 绘制波峰标记
        self.ax.plot(peaks, [data[i] for i in peaks], "rx", label="Peaks")

        # 添加标题和标签
        self.ax.set_title('Peak Area')
        self.ax.set_xlabel('X Values')
        self.ax.set_ylabel('Data Values')

        # 显示图例
        self.ax.legend()

        # 绘制图形
        self.draw()


class TextWindow(QMainWindow):
    def __init__(self):
        super().__init__()

        self.setWindowTitle('Matplotlib in PyQt')

        # 创建一个QWidget作为主窗口
        self.central_widget = QWidget(self)
        self.setCentralWidget(self.central_widget)

        # 创建QVBoxLayout来布局
        self.layout = QVBoxLayout(self.central_widget)

        # 创建一个显示文本的Label
        self.label = QLabel('Peak Area Ratio: ', self)
        self.layout.addWidget(self.label)

        # 创建绘图区域
        self.canvas = PlotCanvas(self)
        self.layout.addWidget(self.canvas)

        # 创建按钮并连接槽函数
        self.button = QPushButton('Calculate and Plot', self)
        self.button.clicked.connect(self.on_button_click)
        self.layout.addWidget(self.button)

        # 创建按钮并连接槽函数
        self.button = QPushButton('返回', self)
        self.button.clicked.connect(self.on_main_click)
        self.layout.addWidget(self.button)

        # 设置固定窗口大小为 470x280
        self.setFixedSize(470, 280)
    def on_button_click(self):
        # 加载数据
        load_data()

        # 计算波峰及其面积
        peaks, peak_areas = calculate_peak_area_base_line(data_values)

        # 绘制图形
        self.canvas.plot(data_values, peaks)

        # 显示面积比值
        peak_area_ratio = calculate_area_ratio(peaks, peak_areas)
        if peak_area_ratio:
            self.label.setText(f"Peak Area Ratio: {peak_area_ratio:.2f}")
        else:
            self.label.setText("Peak Area Ratio: N/A")

    def on_main_click(self):
            self.main_window = MainWindow()  # 创建主窗口对象
            self.main_window.show()
            self.close()  #

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
            self.open_main_window()  # 打开主界面


    def open_main_window(self):
        self.main_window = MainWindow()  # 创建主窗口对象
        self.main_window.show()
        self.close()  #



class WelcomeWindow(QMainWindow):
    def __init__(self):
        super().__init__()
        loadUi("welcome.ui", self)  # 加载 welcome.ui
        self.main_button.clicked.connect(self.show_main_window)  # main.ui 按钮
        self.fluo_button.clicked.connect(self.show_fluo_window)  # FluoTest.ui 按钮
        self.text_button.clicked.connect(self.show_text_window)



    def show_main_window(self):
        self.main_window = MainWindow()  # 创建主窗口对象
        self.main_window.show()
        self.close()  #

    def show_fluo_window(self):
        self.fluo_window = FluoTestWindow()  # 创建 FluoTest 窗口对象
        self.fluo_window.show()
        self.close()  #

    def show_text_window(self):
        self.text_window = TextWindow()  # 创建 FluoTest 窗口对象
        self.text_window.show()
        self.close()  #


class MainWindow(QMainWindow):
    def __init__(self):
        super().__init__()
        loadUi("main.ui", self)  # 加载 main.ui
        self.settingButton.clicked.connect(self.show_fluo_window)
        self.historyButton.clicked.connect(self.show_history_window)
        self.itemButton.clicked.connect(self.show_item_window)
        self.testButton.clicked.connect(self.show_test_window)
        self.close_button.clicked.connect(self.show_close_window)
        self.setbox_button.clicked.connect(self.show_welcome_window)

    def show_fluo_window(self):
        self.fluo_window = FluoTestWindow()  # 创建 FluoTest 窗口对象
        self.fluo_window.show()
        self.close()  # 关闭当前

    def show_welcome_window(self):
        # 创建欢迎界面并显示
        self.welcome_window = WelcomeWindow()
        self.welcome_window.show()
        # 关闭欢迎界面
        self.close()

    def show_history_window(self):
        self.historys_window = HistorysWindow()
        self.historys_window.show()
        self.close()  # 关闭当前

    def show_item_window(self):
        self.item_window = QuerylineWindow()
        self.item_window.show()
        self.close()  # 关闭当前

    def show_test_window(self):
        self.test_window = TextWindow()  # 创建 FluoTest 窗口对象
        self.test_window.show()
        self.close()  #
    def show_close_window(self):
        self.close()  #


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
        self.main_button.clicked.connect(self.show_main_window)
    def show_main_window(self):
        self.main_window = MainWindow()  # 创建主窗口对象
        self.main_window.show()
        self.close()  # 关闭当前的欢迎窗口


class ChannelWindow(QMainWindow):
    def __init__(self):
        super().__init__()
        loadUi("channel.ui", self)  # 加载 ui
        self.channelsetting.clicked.connect(self.show_setting_window)
        self.main_button.clicked.connect(self.show_main_window)

    def show_setting_window(self):
        self.setting_window = ChannelsettingWindow()  # 创建主窗口对象
        self.setting_window.show()
        self.close()  # 关闭当前的欢迎窗口
    def show_main_window(self):
        self.main_window = MainWindow()  # 创建主窗口对象
        self.main_window.show()
        self.close()  # 关闭当前的欢迎窗口


class ChannelsettingWindow(QMainWindow):
    def __init__(self):
        super().__init__()
        loadUi("channelsetting.ui", self)  # 加载 ui
        self.controlButton.clicked.connect(self.show_control_window)
        self.main_button.clicked.connect(self.show_main_window)
    def show_control_window(self):
        self.control_window = ChannelWindow()  # 创建主窗口对象
        self.control_window.show()
        self.close()  # 关闭当前的欢迎窗口
    def show_main_window(self):
        self.main_window = MainWindow()  # 创建主窗口对象
        self.main_window.show()
        self.close()  # 关闭当前的欢迎窗口

if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = Welcome1Window()  # 创建欢迎窗口
    window.show()  # 显示欢迎窗口
    sys.exit(app.exec_())  # 启动应用
