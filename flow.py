
import sys
import numpy as np
import sqlite3
from PyQt5.QtWidgets import (QApplication, QMainWindow, QProgressBar, QComboBox, QTableView, QVBoxLayout, QPushButton,
                             QLineEdit, QLabel, QHBoxLayout, QWidget, QFormLayout, QMessageBox)
from PyQt5.uic import loadUi
from PyQt5.QtCore import Qt, QAbstractTableModel, QTimer
from scipy.signal import find_peaks
import matplotlib.pyplot as plt
from matplotlib.backends.backend_qt5agg import FigureCanvasQTAgg as FigureCanvas
DATABASE = "app.db"




def load_data():
    global data_values
    data_values = []
    # 从文件中读取数据
    try:
        with open('data.txt', 'r') as file:
            hex_data = file.read().split()
            # 跳过前6个数据（根据实际情况调整）
            hex_data = hex_data[6:]
            for i in range(0, len(hex_data), 2):
                hex_pair = hex_data[i:i + 2]
                if len(hex_pair) == 2:
                    low_byte = int(hex_pair[0], 16)
                    high_byte = int(hex_pair[1], 16)
                    value = low_byte + (high_byte << 8)
                    data_values.append(value)
        data_values = np.array(data_values)
    except Exception as e:
        print(f"Error loading data: {e}")

# ----------------------- 检测参数计算 -----------------------
def calculate_detection_parameters(data):
    """
    对传入数据利用 find_peaks 寻峰，
    对每个波峰动态确定左右边界，
    并计算：基线值、波峰宽度、毛面积（原始曲线积分）和净面积（毛面积减去基线面积）。
    """
    peaks, properties = find_peaks(data, prominence=100, width=1)
    peak_params = []
    for peak in peaks:
        left = peak
        right = peak

        # 动态调整左右边界（步长可根据数据特性调整）
        while left > 0 and data[left] > data[left - 1]:
            left = max(0, left - 10)
        while right < len(data) - 1 and data[right] > data[right + 1]:
            right = min(len(data) - 1, right + 10)

        # 扩展检测范围用于基线计算
        left_extended = max(0, left - 100)
        right_extended = min(len(data) - 1, right + 100)

        # 分别计算左右侧的均值作为基线候选
        baseline_left = np.mean(data[left_extended:peak + 1])
        baseline_right = np.mean(data[peak:right_extended + 1])
        baseline = min(baseline_left, baseline_right)

        # 波峰宽度（数据点数，可乘以采样间隔得到实际宽度）
        width = right - left

        # 毛面积：在左右边界内对原始信号积分
        gross_area = np.trapz(data[left:right + 1], dx=1)
        # 基线面积
        baseline_area = baseline * (right - left)
        # 净面积：荧光面积（毛面积减去基线面积）
        net_area = gross_area - baseline_area

        peak_params.append({
            'peak_index': peak,
            'left_index': left,
            'right_index': right,
            'baseline': baseline,
            'width': width,
            'gross_area': gross_area,
            'net_area': net_area
        })
    return peaks, peak_params


# ----------------------- 面积比计算 -----------------------
def get_max_peak_in_range(peak_params, x_range):
    """
    从给定范围内的波峰中，选择净面积最大的波峰
    """
    valid_peaks = [p for p in peak_params if x_range[0] <= p['peak_index'] <= x_range[1]]
    if len(valid_peaks) == 0:
        return None
    return max(valid_peaks, key=lambda p: p['net_area'])

def calculate_area_ratio(peak_params):
    """
    分别在500-700和700-900区间内选择净面积最大的波峰，
    并计算两者净面积的比值
    """
    peak_500_700 = get_max_peak_in_range(peak_params, (500, 700))
    peak_700_900 = get_max_peak_in_range(peak_params, (700, 900))
    if peak_500_700 and peak_700_900:
        print(f"Max peak in range 500-700: Peak {peak_500_700['peak_index']} with net area: {peak_500_700['net_area']}")
        print(f"Max peak in range 700-900: Peak {peak_700_900['peak_index']} with net area: {peak_700_900['net_area']}")
        ratio = peak_500_700['net_area'] / peak_700_900['net_area']
        print(f"The ratio of net peak area in 500-700 to 700-900 is: {ratio}")
        return ratio, peak_500_700, peak_700_900
    else:
        print("One or both of the ranges do not contain any peaks.")
        return None, None, None
# ----------------------- 检测结果数据库 -----------------------

def create_app_database():
    try:
        conn = sqlite3.connect(DATABASE)
        cursor = conn.cursor()
        # 创建用户表
        cursor.execute('''
            CREATE TABLE IF NOT EXISTS user_table (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                username TEXT NOT NULL,
                email TEXT NOT NULL,
                age INTEGER
            )
        ''')
        # 创建检测结果表，关联 user_id
        cursor.execute('''
              CREATE TABLE IF NOT EXISTS detection_results (
                  id INTEGER PRIMARY KEY AUTOINCREMENT,
                  user_id INTEGER,

                  -- 第一个波峰
                  peak_index_1 INTEGER,
                  net_area_1 REAL,

                  -- 第二个波峰
                  peak_index_2 INTEGER,
                  net_area_2 REAL,

                  -- 两个波峰面积比
                  ratio REAL,

                  timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,

                  FOREIGN KEY(user_id) REFERENCES user_table(id)
              )
          ''')
        conn.commit()
        conn.close()
    except Exception as e:
        print(f"Error creating app database: {e}")

def insert_user(username, email, age):
    """插入用户信息（包含年龄）"""
    try:
        conn = sqlite3.connect(DATABASE)
        cursor = conn.cursor()
        cursor.execute('INSERT INTO user_table (username, email, age) VALUES (?, ?, ?)', (username, email, age))
        conn.commit()
        conn.close()
    except Exception as e:
        print(f"Error inserting user: {e}")


def store_two_peaks(peak_500_700, peak_700_900, ratio, user_id=0):
    """
    一条记录同时存储两个波峰信息 + 面积比
    """
    try:
        conn = sqlite3.connect(DATABASE)
        cursor = conn.cursor()

        cursor.execute('''
            INSERT INTO detection_results 
            (user_id,
             peak_index_1, net_area_1,
             peak_index_2, net_area_2,
             ratio
            )
            VALUES (?, ?, ?, ?, ?, ?)
        ''', (
            user_id,
            int(peak_500_700['peak_index']), float(peak_500_700['net_area']),
            int(peak_700_900['peak_index']), float(peak_700_900['net_area']),
            float(ratio)
        ))
        conn.commit()
        conn.close()

        print("成功插入组合波峰记录到 detection_results 表。")
    except Exception as e:
        print(f"Error inserting two peaks result: {e}")

def store_detection_results(result, user_id=0):
    try:
        conn = sqlite3.connect(DATABASE)
        cursor = conn.cursor()
        cursor.execute('''
            INSERT INTO detection_results (user_id, peak_index, left_index, right_index, baseline, peak_width, gross_area, net_area)
            VALUES (?, ?, ?, ?, ?, ?, ?, ?)
        ''', (user_id,
              int(result['peak_index']),    # 显式转换为内置 int
              int(result['left_index']),
              int(result['right_index']),
              float(result['baseline']),
              float(result['width']),
              float(result['gross_area']),
              float(result['net_area'])))
        conn.commit()
        conn.close()
    except Exception as e:
        print(f"Error inserting detection result: {e}")

# 查询用户数据
def fetch_users(columns=[]):
    try:
        conn = sqlite3.connect(DATABASE)
        cursor = conn.cursor()
        column_list = ", ".join(columns) if columns else "*"
        cursor.execute(f'SELECT {column_list} FROM user_table')
        rows = cursor.fetchall()
        conn.close()
        return rows
    except Exception as e:
        print(f"Error fetching users: {e}")
        return []


# 表格模型
class TableModel(QAbstractTableModel):
    def __init__(self, data):
        super().__init__()
        self._data = data

    def rowCount(self, parent=None):
        return len(self._data)

    def columnCount(self, parent=None):
        return len(self._data[0]) if self._data else 0

    def data(self, index, role):
        if role == Qt.DisplayRole:
            return str(self._data[index.row()][index.column()])
        return None

    def headerData(self, section, orientation, role):
        if role == Qt.DisplayRole and orientation == Qt.Horizontal:
            headers = ['ID', 'Username', 'Email', 'Age']
            return headers[section] if section < len(headers) else str(section + 1)
        if role == Qt.DisplayRole and orientation == Qt.Vertical:
            return str(section + 1)
        return None



class DetectionResultsWindow(QMainWindow):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("Detection Results")
        self.setFixedSize(700, 400)  # 你可以调整大小

        # 创建表格视图
        self.table_view = QTableView(self)

        # 创建一个按钮，用来刷新数据或者返回
        self.refresh_button = QPushButton("Refresh / Reload", self)
        self.close_button = QPushButton("Close", self)

        # 将按钮和表格布局
        layout = QVBoxLayout()
        layout.addWidget(self.table_view)

        button_layout = QHBoxLayout()
        button_layout.addWidget(self.refresh_button)
        button_layout.addWidget(self.close_button)
        layout.addLayout(button_layout)

        # 创建一个容器并设置为中央Widget
        container = QWidget()
        container.setLayout(layout)
        self.setCentralWidget(container)

        # 按钮事件
        self.refresh_button.clicked.connect(self.refresh_table)
        self.close_button.clicked.connect(self.close)

        # 启动时就加载一次数据
        self.refresh_table()

    # 在 DetectionResultsWindow 中:
    def refresh_table(self):
        rows = fetch_detection_results()
        # 对应 SELECT 的列顺序
        headers = [
            "ID", "UserID",
            "PeakIndex_1", "NetArea_1",
            "PeakIndex_2", "NetArea_2",
            "Ratio",
            "Timestamp"
        ]
        model = DetectionTableModel(rows, headers)
        self.table_view.setModel(model)


def fetch_detection_results():
    """
    现在 detection_results 里只有:
      id, user_id,
      peak_index_1, net_area_1,
      peak_index_2, net_area_2,
      ratio,
      timestamp
    """
    try:
        conn = sqlite3.connect(DATABASE)
        cursor = conn.cursor()
        cursor.execute('''
            SELECT id, user_id,
                   peak_index_1, net_area_1,
                   peak_index_2, net_area_2,
                   ratio,
                   timestamp
            FROM detection_results
        ''')
        rows = cursor.fetchall()
        conn.close()
        return rows
    except Exception as e:
        print(f"Error fetching detection results: {e}")
        return []

class DetectionTableModel(QAbstractTableModel):
    def __init__(self, data, headers):
        super().__init__()
        self._data = data
        self._headers = headers

    def rowCount(self, parent=None):
        return len(self._data)

    def columnCount(self, parent=None):
        return len(self._headers)

    def data(self, index, role=Qt.DisplayRole):
        if role == Qt.DisplayRole:
            row = index.row()
            col = index.column()
            return str(self._data[row][col])
        return None

    def headerData(self, section, orientation, role=Qt.DisplayRole):
        if role == Qt.DisplayRole:
            if orientation == Qt.Horizontal:
                return self._headers[section]
            else:
                return str(section + 1)
        return None



# ----------------------- 绘图类 -----------------------
class PlotCanvas(FigureCanvas):
    def __init__(self, parent=None):
        self.fig, self.ax = plt.subplots(figsize=(4.7, 2.8))
        super().__init__(self.fig)
        self.setParent(parent)

    def plot(self, data, peaks, selected_peaks=None):
        self.ax.clear()
        self.ax.plot(range(len(data)), data, label='Original Data')
        self.ax.plot(peaks, [data[i] for i in peaks], "rx", label="Peaks")
        if selected_peaks:
            for sp in selected_peaks:
                left = sp['left_index']
                right = sp['right_index']
                baseline = sp['baseline']
                self.ax.axvline(x=left, color="orange", linestyle="--", label="Left Boundary" if sp==selected_peaks[0] else "")
                self.ax.axvline(x=right, color="purple", linestyle="--", label="Right Boundary" if sp==selected_peaks[0] else "")
                self.ax.axhline(y=baseline, color="green", linestyle="--", label="Baseline" if sp==selected_peaks[0] else "")
        self.ax.set_title('Peak Analysis')
        self.ax.set_xlabel('Data Points')
        self.ax.set_ylabel('Signal')
        self.ax.legend()
        self.draw()


# ----------------------- 检测结果界面 -----------------------
class TextWindow(QMainWindow):
    def __init__(self, user_id=0):
        super().__init__()
        self.user_id = user_id  # 保存所选用户的id

        self.setWindowTitle('Fluorescence Detection Analysis')
        self.central_widget = QWidget(self)
        self.setCentralWidget(self.central_widget)
        self.layout = QVBoxLayout(self.central_widget)

        self.label = QLabel('Peak Area Ratio: ', self)
        self.layout.addWidget(self.label)

        self.canvas = PlotCanvas(self)
        self.layout.addWidget(self.canvas)

        self.calc_button = QPushButton('Calculate and Plot', self)
        self.calc_button.clicked.connect(self.on_button_click)
        self.layout.addWidget(self.calc_button)

        self.back_button = QPushButton('返回', self)
        self.back_button.clicked.connect(self.on_main_click)
        self.layout.addWidget(self.back_button)

        self.setFixedSize(470, 280)

    def on_button_click(self):
        load_data()
        if data_values.size == 0:
            QMessageBox.warning(self, "警告", "数据加载失败或数据为空")
            return

        # 计算所有波峰的检测参数
        peaks, peak_params = calculate_detection_parameters(data_values)
        # 分别在两个区间选择净面积最大的波峰，并计算面积比
        ratio, peak_500_700, peak_700_900 = calculate_area_ratio(peak_params)

        # 绘图时标出所有波峰，并用不同颜色标出用于面积比计算的波峰
        selected_peaks = [p for p in [peak_500_700, peak_700_900] if p is not None]
        self.canvas.plot(data_values, peaks, selected_peaks=selected_peaks)

        # 显示面积比并存储数据库
        if ratio is not None:
            self.label.setText(f"Peak Area Ratio: {ratio:.2f}\n"
                               f"500-700 peak net area: {peak_500_700['net_area']:.2f}\n"
                               f"700-900 peak net area: {peak_700_900['net_area']:.2f}")


            store_two_peaks(peak_500_700, peak_700_900, ratio, self.user_id)
        else:
            self.label.setText("Peak Area Ratio: N/A")

    def on_main_click(self):
        self.main_window = MainWindow()
        self.main_window.show()
        self.close()


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
        self.data_Button.clicked.connect(self.show_user_window)
        # 设置按钮点击事件
        self.testButton.clicked.connect(self.show_test_window)
        ...

        # 调用加载用户列表
        self.load_users_to_combobox()
    def load_users_to_combobox(self):
        """
        从数据库 user_table 中获取所有用户，将其显示在 userComboBox 中。
        """
        users = fetch_users()  # 通常返回 [(id, username, email), ...]
        self.userComboBox.clear()
        if not users:
            # 如果没有任何用户，就加入一个占位项
            self.userComboBox.addItem("（无用户）", -1)
        else:
            for row in users:
                user_id = row[0]  # id
                username = row[1]  # username
                # 在ComboBox里显示username，同时把user_id作为对应数据
                self.userComboBox.addItem(username, user_id)

    def show_fluo_window(self):
        self.fluo_window = FluoTestWindow()  # 创建 FluoTest 窗口对象
        self.fluo_window.show()
        self.close()  # 关闭当前

    def show_user_window(self):
        # 创建欢迎界面并显示
        self.user_window = UserWindow()
        self.user_window.show()
        # 关闭欢迎界面
        self.close()

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
        # 1) 从下拉框获取当前选中的 user_id
        selected_user_id = self.userComboBox.currentData()
        if selected_user_id is None or selected_user_id < 0:
            QMessageBox.warning(self, "警告", "请先在下拉框里选择一个用户！")
            return

        # 2) 创建 TextWindow 的时候，把 user_id 作为参数传过去
        self.test_window = TextWindow(user_id=selected_user_id)
        self.test_window.show()
        self.close()

    def show_close_window(self):
        self.close()  #

class UserWindow(QMainWindow):
    def __init__(self):
        super().__init__()

        self.setWindowTitle("User Data Management")
        self.setFixedSize(470, 280)  # 设置窗口固定大小为 470x280

        # 创建UI组件
        self.table_view = QTableView(self)
        self.load_user_button = QPushButton("Load Users from File", self)
        self.insert_user_button = QPushButton("Insert User", self)
        self.query_button = QPushButton("Query Users", self)

        self.username_input = QLineEdit(self)
        self.email_input = QLineEdit(self)
        self.age_input = QLineEdit(self)

        self.username_label = QLabel("Username: ", self)
        self.email_label = QLabel("Email: ", self)
        self.age_label = QLabel("Age: ", self)
        # --- 新增一个返回主界面的按钮 ---
        self.back_button = QPushButton("返回主界面", self)  # 按钮文字可自定义
        self.query_combobox = QComboBox(self)
        # 创建左侧布局（输入区和按钮）
        left_layout = QVBoxLayout()
        left_layout.addWidget(self.load_user_button)
        left_layout.addWidget(self.insert_user_button)

        left_layout.addWidget(self.age_label)
        left_layout.addWidget(self.age_input)
        left_layout.addWidget(self.username_label)
        left_layout.addWidget(self.username_input)
        left_layout.addWidget(self.email_label)
        left_layout.addWidget(self.email_input)
        # 加在下方
        left_layout.addWidget(self.back_button)  # <-- 这里添加“返回主界面”按钮

        # 最后再加一个 “View Detection Results” 按钮
        self.view_detection_button = QPushButton("View Detection Results", self)
        left_layout.addWidget(self.view_detection_button)
        # 创建右侧布局（查询部分和表格）
        right_layout = QVBoxLayout()
        right_layout.addWidget(self.query_button)
        right_layout.addWidget(self.query_combobox)
        right_layout.addWidget(self.table_view)

        # 创建水平布局，将左侧和右侧布局放在一行
        main_layout = QHBoxLayout()
        main_layout.addLayout(left_layout, stretch=1)  # 左侧占 2/3
        main_layout.addLayout(right_layout, stretch=2)  # 右侧占 1/3

        # 设定主布局
        container = QWidget()
        container.setLayout(main_layout)
        self.setCentralWidget(container)

        # 再绑定事件
        self.view_detection_button.clicked.connect(self.show_detection_window)
        # 按钮点击事件
        self.back_button.clicked.connect(self.on_back_to_main)  # <-- 绑定事件
        self.load_user_button.clicked.connect(self.load_users_from_file)
        self.insert_user_button.clicked.connect(self.insert_user)
        self.query_button.clicked.connect(self.query_users)

    def load_users_from_file(self):
        # 假设从文件中读取用户数据并插入到数据库
        # 这里你可以用实际的文件路径
        self.refresh_table()

    def on_back_to_main(self):
        """
        返回主界面
        """
        self.main_window = MainWindow()
        self.main_window.show()
        self.close()

    def show_detection_window(self):
        self.det_window = DetectionResultsWindow()

        self.det_window.show()

    def insert_user(self):
        # 从输入框中获取数据
        username = self.username_input.text()
        email = self.email_input.text()
        age_text = self.age_input.text()
        try:
            age = int(age_text)
        except:
            QMessageBox.warning(self, "警告", "请输入有效的年龄（整数）！")
            return

        insert_user(username, email, age)
        self.username_input.clear()
        self.email_input.clear()
        self.age_input.clear()
        self.refresh_table()

    def query_users(self):
        selected_column = self.query_combobox.currentText()
        if selected_column == "All":
            columns = []
        else:
            columns = [selected_column]
        users = fetch_users(columns)
        self.refresh_table(users)

    def refresh_table(self, users=None):
        # 更新表格
        if users is None:
            users = fetch_users()

        model = TableModel(users)
        self.table_view.setModel(model)

        # 更新查询选项
        self.query_combobox.clear()
        self.query_combobox.addItem("All")
        self.query_combobox.addItems([col[1] for col in self.get_columns()])

    def get_columns(self):
        # 获取所有列名，从统一的数据库中查询
        conn = sqlite3.connect(DATABASE)  # 改为使用 DATABASE
        cursor = conn.cursor()
        cursor.execute('PRAGMA table_info(user_table)')
        columns = cursor.fetchall()
        conn.close()
        return columns




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
    create_app_database()  # 创建所有表（包括 detection_results）
    app = QApplication(sys.argv)
    window = Welcome1Window()  # 创建欢迎窗口
    window.show()  # 显示欢迎窗口
    sys.exit(app.exec_())

