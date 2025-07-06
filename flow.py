from PyQt5.QtWidgets import QApplication, QMainWindow, QProgressBar, QVBoxLayout, QWidget, QPushButton,QMessageBox
from PyQt5.uic import loadUi
from PyQt5.QtCore import Qt, QAbstractTableModel, QTimer
import sqlite3
from control import get_serial_data  # 导入获取串口数据的函数
import sys
import paho.mqtt.client as mqtt
import ssl
import json
from PyQt5.QtCore import Qt
from PyQt5.QtGui import QPainter, QColor
from PyQt5.QtWidgets import QWidget
import numpy as np

DATABASE = "app.db"
# 华为云 MQTT 配置信息（请替换为实际值）
HW_MQTT_BROKER = "cb52aa03b3.st1.iotda-device.cn-north-4.myhuaweicloud.com"  # 如："xxxx.iot-mqtts.cn-north-4.myhuaweicloud.com"
HW_MQTT_PORT = 1883                        # 通常使用 TLS 端口 8883
DEVICE_ID = "67d68442375e694aa6932228_quanzhiv3s_0_0_2025031608"                  # 设备ID
MQTT_USERNAME = "67d68442375e694aa6932228_quanzhiv3s"              # 用户名（可能为设备名称）
MQTT_PASSWORD = "5f4d6625ecc954dbcb3618102b44dd421cf9235856538f70a7736472bc7ac21a"  # 密码（设备密钥）
# 根据华为云平台要求设置 Topic，以下为示例格式
MQTT_TOPIC = f"$oc/devices/{DEVICE_ID}/sys/properties/report"
CA_CERT_PATH = ""                     # CA 证书路径，如需要验证服务器证书

# 初始化全局 MQTT 客户端
def init_mqtt_client():
    client = mqtt.Client(client_id=DEVICE_ID, clean_session=True)
    client.username_pw_set(MQTT_USERNAME, MQTT_PASSWORD)
    if CA_CERT_PATH:
        client.tls_set(
            CA_CERT_PATH,
            certfile=None,
            keyfile=None,
            cert_reqs=ssl.CERT_REQUIRED,
            tls_version=ssl.PROTOCOL_TLS,
            ciphers=None
        )
        client.tls_insecure_set(False)

    # 回调函数（可根据需要扩展）
    client.on_connect = lambda c, u, f, rc: print("MQTT connected with code", rc)
    client.on_publish = lambda c, u, mid: print("MQTT published message id:", mid)
    try:
        client.connect(HW_MQTT_BROKER, HW_MQTT_PORT, keepalive=60)
    except Exception as e:
        print("Error connecting to MQTT broker:", e)
    client.loop_start()  # 启动网络循环
    return client

# 全局保存 MQTT 客户端对象
mqtt_client = init_mqtt_client()

def upload_detection_to_huawei_cloud(result, ratio):
    """
    将检测结果通过 MQTT 协议上传到华为云平台
    参数：
      result：字典，包含两个波峰的信息（例如 peak_index、net_area 等）
      ratio：两个波峰面积比
    """
    # 从 result 中取出 user_id，并获取用户记录
    user_id = result.get("user_id", 0)
    user = get_user_by_id(user_id)
    if user:
        username = user[1]  # 用户姓名
        age = user[3]       # 年龄
    else:
        username = ""
        age = 0
        # 这里构造属性上报的 JSON 格式
    payload = {
        "services": [
            {
                "service_id": "flow",  # 修改为 "flow"
                "properties": {
                    "user_id": str(user_id),
                    "age": age,
                    "peak_index_1": int(result["peak_index_1"]),
                    "net_area_1": round(float(result["net_area_1"]), 4),  # 保留四位小数
                    "peak_index_2": int(result["peak_index_2"]),
                    "net_area_2": round(float(result["net_area_2"]), 4),  # 保留四位小数
                    "ratio": round(float(ratio), 4),
                    "username": str(username)
                }
            }
        ]
    }
    payload_str = json.dumps(payload)
    ret = mqtt_client.publish(MQTT_TOPIC, payload=payload_str, qos=1)
    if ret.rc == mqtt.MQTT_ERR_SUCCESS:
        print("属性上报成功，payload =", payload_str)
    else:
        print("属性上报失败，返回码：", ret.rc)

def load_data():
    global data_values
    global data_valuess
    data_values = []
    try:
        with open('data.txt', 'r') as file:
            content = file.read()  # 可以先读取整个内容
            print("成功读取 datas.txt，字符数：", len(content))
            hex_data = content.split()
            hex_data = hex_data[6:]
            for i in range(0, len(hex_data), 2):
                hex_pair = hex_data[i:i + 2]
                if len(hex_pair) == 2:
                    low_byte = int(hex_pair[0], 16)
                    high_byte = int(hex_pair[1], 16)
                    value = low_byte + (high_byte << 8)
                    data_values.append(value)
        data_values = np.array(data_values)
        print("data_values 长度：", len(data_values))
    except Exception as e:
        print(f"Error loading data: {e}")


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
        self.close()  # 关闭当前的欢迎窗口


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
        self.data_Button.clicked.connect(self.show_user_window)
        self.testButton.clicked.connect(self.show_test_window)
        self.view_detection_button.clicked.connect(self.show_detection_window)
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

    def show_detection_window(self):
        self.det_window = DetectionResultsWindow()
        self.det_window.show()
        self.close()

    def show_test_window(self):
        self.test_window = TextWindow()  # 创建 FluoTest 窗口对象
        self.test_window.show()
        self.close()  # 关闭当前
class FluoTestWindow(QMainWindow):
    def __init__(self):
        super().__init__()
        loadUi("FluoTest.ui", self)  # 加载 FluoTest.ui
        self.mainbutton.clicked.connect(self.show_main_window)
        self.userbutton.clicked.connect(self.show_test_window)
        # 调用加载用户列表
        self.load_users_to_combobox()
        # FluoTest.ui 按钮
    def show_main_window(self):
        self.main_window = MainWindow()  # 创建主窗口对象
        self.main_window.show()
        self.close()  # 关闭当前的欢迎窗口

    def show_test_window(self):
        user_id = self.userComboBox.currentData()  # 获取当前选择的用户ID
        self.test_window = TextWindow(user_id=user_id)  # 将正确的 user_id 传递过去
        self.test_window.show()
        self.close()  # 关闭当前窗口


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

                self.userComboBox.addItem(str(user_id), user_id)


class PlotCanvas(QWidget):
    def __init__(self, parent=None):
        super().__init__(parent)
        self.setWindowTitle("Peak Analysis")

        # 初始化数据
        self.data = None
        self.peaks = None
        self.selected_peaks = None

    def plot(self, data, peaks, selected_peaks=None):
        # 将数据和峰值传递给对象
        self.data = data
        self.peaks = peaks
        self.selected_peaks = selected_peaks
        self.update()  # 刷新窗口，触发 paintEvent

    def paintEvent(self, event):
        painter = QPainter(self)
        painter.setRenderHint(QPainter.Antialiasing)  # 启用抗锯齿

        if self.data is not None:
            # 获取数据的最小值和最大值
            min_data = min(self.data)
            max_data = max(self.data)

            # 设置一定的边距
            y_margin = 50  # Y 轴的边距
            y_range = max_data - min_data + 2 * y_margin  # Y 轴的显示范围

            # 设置 X 轴范围
            x_range = len(self.data)

            # 绘制数据线，使用动态的坐标范围
            for i in range(len(self.data) - 1):
                # 缩放数据坐标
                x1 = (i / x_range) * self.width()
                y1 = self.height() - (self.data[i] - min_data + y_margin) / y_range * self.height()  # 翻转 y 坐标
                x2 = ((i + 1) / x_range) * self.width()
                y2 = self.height() - (self.data[i + 1] - min_data + y_margin) / y_range * self.height()  # 翻转 y 坐标

                # 绘制线条
                painter.setPen(QColor(0, 0, 0))  # 黑色线条
                painter.drawLine(int(x1), int(y1), int(x2), int(y2))

            # 绘制峰值（红色叉号）
            if self.peaks is not None:
                for peak in self.peaks:
                    x = (peak / len(self.data)) * self.width()  # 计算峰的 x 坐标
                    y = self.height() - (self.data[peak] - min_data + y_margin) / y_range * self.height()  # 翻转 y 坐标
                    painter.setPen(QColor(255, 0, 0))  # 红色
                    painter.drawLine(x - 5, y - 5, x + 5, y + 5)
                    painter.drawLine(x - 5, y + 5, x + 5, y - 5)

        painter.end()  # 结束绘制

class TextWindow(QMainWindow):
    def __init__(self, user_id=0):
        super().__init__()
        loadUi("text.ui", self)
        self.user_id = user_id  # 保存所选用户的id

        self.canvas = PlotCanvas(self)  # 创建绘图画布
        self.layout().addWidget(self.canvas)  # 将画布添加到UI布局中

        self.canvas.setGeometry(50, 50, 680, 420)  # 调整画布大小

        self.calc_button.clicked.connect(self.on_button_click)

        self.back_button.clicked.connect(self.on_main_click)

        #self.is_plotting = False  # 标志位，确保绘图操作不会重复触发
    def on_button_click(self):
      #  if self.is_plotting:  # 如果正在绘图，直接返回，避免重复触发
          #  return

      # self.is_plotting = True  # 设置标志，防止多次绘图操作
        # 1) 获取串口数据
        data_values = self.get_data_from_serial()

        #load_data()
        # 如果获取到数据，则处理
        #if data_values is None or len(data_values) == 0:
          #  QMessageBox.warning(self, "警告", "数据加载失败或数据为空")
           # self.is_plotting = False  # 绘图完成，重置标志
          #  return

        print("原始数据：min =", data_values.min(), "max =", data_values.max())
        smoothed = smooth_data(data_values, window_size=9)
        print("平滑后数据：min =", smoothed.min(), "max =", smoothed.max())

        # 计算所有波峰的检测参数
        # 检测峰，使用自定义函数

        peaks = custom_find_peaks(smoothed, prominence=5, min_distance=60)
        print("检测到的峰索引：", peaks)
        for p in peaks:
            print(f"索引 {p}，值 = {smoothed[p]}")
        # 4) 计算每个峰的净面积等参数
        peak_params = refined_calculate_peak_params(
            data_values,
            peaks,
            step=2,
            extension=74,
            slope_threshold=5.0
        )
        # 5) 计算两峰面积比
        ratio, peak_500_700, peak_700_900 = calculate_area_ratio(peak_params, smoothed)

        # 6) 绘图
        selected_peaks = [p for p in [peak_500_700, peak_700_900] if p is not None]
        # 将数据传递给画布进行绘制

        self.canvas.update()  # 强制刷新绘图区域
        self.canvas.plot(data_values, peaks, selected_peaks=selected_peaks)
        print("Detected peaks:", peaks)
        for p in peak_params:
            print(
                f"Peak at {p['peak_index']} => net_area={p['net_area']}, baseline={p['baseline']}, left={p['left_index']}, right={p['right_index']}")

        # 显示面积比并存储数据库
        if ratio is not None:
            self.label.setText(f"峰面积比          : {ratio:.2f}\n"
                               f"500 - 700 峰净面积: {peak_500_700['net_area']:.2f}\n"
                               f"700 - 900 峰净面积: {peak_700_900['net_area']:.2f}")

            # 存储组合波峰记录并上传至华为云
            store_two_peaks(peak_500_700, peak_700_900, ratio, self.user_id)
            self.is_plotting = False  # 绘图完成，重置标志
        else:
            self.label.setText("峰面积比: N/A")


    def get_data_from_serial(self):

        raw_data = get_serial_data()  # 调用控制脚本的函数获取数据
        if raw_data:
            # 使用原来 load_data() 中的处理逻辑
            return self.load_data_from_raw(raw_data)
        else:
            return None


    def load_data_from_raw(self, raw_data):

        hex_data = list(raw_data)
        hex_data = hex_data[6:]  # 裁剪掉前6个字节（根据需求调整）

        data_values = []
        for i in range(0, len(hex_data), 2):
            # 每次取两个字节（防止越界）
            if i + 1 >= len(hex_data):
                break
            byte_low = hex_data[i]  # 直接获取字节整数值
            byte_high = hex_data[i + 1]

            # 组合为16位整数（小端序：低字节在前，高字节在后）
            combined_value = (byte_low << 8) | byte_high
            data_values.append(combined_value)

        return np.array(data_values)


    def on_main_click(self):
        self.main_window = MainWindow()
        self.main_window.show()
        self.close()
        self.deleteLater()


class DetectionResultsWindow(QMainWindow):
    def __init__(self):
        super().__init__()
        loadUi("detection.ui", self)
        self.refresh_button.clicked.connect(self.refresh_table)
        self.close_button.clicked.connect(self.show_main_window)
        # 启动时就加载一次数据
        self.refresh_table()
    def show_main_window(self):
        self.main_window = MainWindow()  # 创建主窗口对象
        self.main_window.show()
        self.close()  # 关闭当前的欢迎窗口
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

class UserWindow(QMainWindow):
    def __init__(self):
        super().__init__()
        loadUi("user.ui", self)
        # 再绑定事件
    #    self.view_detection_button.clicked.connect(self.show_detection_window)
        # 按钮点击事件
        self.back_button.clicked.connect(self.on_back_to_main)  # <-- 绑定事件
        self.load_user_button.clicked.connect(self.load_users_from_file)
        self.insert_user_button.clicked.connect(self.insert_user)
        self.query_button.clicked.connect(self.query_users)

    def load_users_from_file(self):
        # 从文件中读取用户数据并插入到数据库
        self.refresh_table()

    def on_back_to_main(self):
        """
        返回主界面
        """
        self.main_window = MainWindow()
        self.main_window.show()
        self.close()

    def insert_user(self):
        # 从输入框中获取数据
        username = self.username_input.text()
        email = self.email_input.text()
        age_text = self.age_input.text()


        insert_user(username, email, age_text)
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
    将两个波峰信息和面积比整合为一条记录存入本地数据库，
    然后调用属性上报函数上传到华为云平台。
    存储字段包括：peak_index_1, net_area_1, peak_index_2, net_area_2, ratio
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
        # 为上传到华为云准备数据
        upload_data = {
            "user_id": user_id,
            "peak_index_1": int(peak_500_700['peak_index']),
            "net_area_1": float(peak_500_700['net_area']),
            "peak_index_2": int(peak_700_900['peak_index']),
            "net_area_2": float(peak_700_900['net_area'])
        }

        # 添加 user_id 到上传数据中，用于属性上报
        upload_data["user_id"] = user_id
        # 上传属性数据
        upload_detection_to_huawei_cloud(upload_data, ratio)
    except Exception as e:
        print(f"Error inserting two peaks result: {e}")


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



# ----------------------- 面积比计算 -----------------------
def get_max_peak_in_range(peak_params, x_range):
    """
    从给定范围内的波峰中，选择净面积最大的波峰
    """
    valid_peaks = [p for p in peak_params if x_range[0] <= p['peak_index'] <= x_range[1]]
    if len(valid_peaks) == 0:
        return None
    return max(valid_peaks, key=lambda p: p['net_area'])


def calculate_area_ratio(peak_params, data):
    """
    根据给定的 peak_params 和原始 data，
    在预定的两个区间 [500,700] 和 [700,900] 内选择净面积最大的峰，
    若未能直接检测到两个峰，则尝试对跨界峰进行拆分。

    返回：
       (ratio, peak_500_700, peak_700_900)
    """
    # 先按原来的区间规则尝试选择
    peak_500_700 = get_max_peak_in_range(peak_params, (500, 700))
    peak_700_900 = get_max_peak_in_range(peak_params, (700, 900))

    # 如果有任一区间没有峰，但存在一个峰的左右边界跨越了 700，则拆分这个峰
    if (not peak_500_700 or not peak_700_900):
        for p in peak_params:
            if p['left_index'] < 700 and p['right_index'] > 700:
                # 将该峰拆分为左右两个子峰
                # 左侧区域：[p['left_index'], 700]
                left_indices = np.arange(p['left_index'], 701)  # 包含 700
                left_area = np.trapz(data[left_indices], dx=1)
                left_width = 700 - p['left_index']
                left_net = left_area - p['baseline'] * left_width

                # 右侧区域：[700, p['right_index']]
                right_indices = np.arange(700, p['right_index'] + 1)
                right_area = np.trapz(data[right_indices], dx=1)
                right_width = p['right_index'] - 700
                right_net = right_area - p['baseline'] * right_width

                # 构造两个伪峰信息（复制原有峰信息，并更新部分字段）
                pseudo_left = p.copy()
                pseudo_left['peak_index'] = (p['left_index'] + 700) // 2
                pseudo_left['net_area'] = left_net
                pseudo_left['left_index'] = p['left_index']
                pseudo_left['right_index'] = 700

                pseudo_right = p.copy()
                pseudo_right['peak_index'] = (700 + p['right_index']) // 2
                pseudo_right['net_area'] = right_net
                pseudo_right['left_index'] = 700
                pseudo_right['right_index'] = p['right_index']

                peak_500_700 = pseudo_left
                peak_700_900 = pseudo_right
                print("Split a cross-boundary peak into two parts:")
                print(f"  Left part: Peak index ~{pseudo_left['peak_index']}, net_area={pseudo_left['net_area']}")
                print(f"  Right part: Peak index ~{pseudo_right['peak_index']}, net_area={pseudo_right['net_area']}")
                break

    if peak_500_700 and peak_700_900:
        if peak_700_900['net_area'] == 0:
            ratio = None
        else:
            ratio = peak_500_700['net_area'] / peak_700_900['net_area']
        print(f"Max peak in range 500-700: Peak {peak_500_700['peak_index']} with net area: {peak_500_700['net_area']}")
        print(f"Max peak in range 700-900: Peak {peak_700_900['peak_index']} with net area: {peak_700_900['net_area']}")
        print(f"The ratio of net peak area in 500-700 to 700-900 is: {ratio}")
        return ratio, peak_500_700, peak_700_900
    else:
        print("One or both of the ranges do not contain any peaks.")
        return None, None, None

# ----------------------- 检测参数计算 -----------------------
def find_peak_boundaries(data, peaks):
    """
    根据相邻峰中点，给每个峰分配一个最大搜索区间 [left_limit, right_limit]。
    peaks: 已按升序排序好的峰顶索引 list 或 np.array
    返回: boundary_limits = [(left_limit_i, right_limit_i), ...] 与 peaks 一一对应
    """
    n = len(data)
    boundaries = []

    for i, p in enumerate(peaks):
        # 左侧限制
        if i == 0:
            left_limit = 0
        else:
            # 与前一个峰的中点
            prev_peak = peaks[i - 1]
            left_limit = (prev_peak + p) // 2

        # 右侧限制
        if i == len(peaks) - 1:
            right_limit = n - 1
        else:
            # 与下一个峰的中点
            next_peak = peaks[i + 1]
            right_limit = (p + next_peak) // 2

        boundaries.append((left_limit, right_limit))

    return boundaries


#############################
# 2) 修改 refined_calculate_peak_params
#############################
def refined_calculate_peak_params(data, peaks, step=5, extension=50, slope_threshold=5.0):
    """
    改进版的峰参数计算：
      - 先用 find_peak_boundaries 确定每个峰的最大搜索区间
      - 在各自区间内向左右搜索边界
      - 局部基线计算

    参数:
      data, peaks, step, extension, slope_threshold 同之前
    """
    data = np.asarray(data, dtype=np.float32)
    n = len(data)
    if len(peaks) == 0:
        return []

    # 先保证 peaks 升序
    peaks = sorted(peaks)

    # 1) 根据相邻峰中点，得到每个峰的最大搜索区
    boundary_limits = find_peak_boundaries(data, peaks)

    peak_params = []
    for i, peak in enumerate(peaks):
        left_limit, right_limit = boundary_limits[i]

        # 2) 从峰顶往左找边界，但不能越过 left_limit
        left = peak
        while left > left_limit:
            if data[left] > data[left - 1]:
                left -= step
            else:
                if (data[left] - data[left - 1]) > slope_threshold:
                    left -= step
                else:
                    break
            if left < 0:
                left = 0
                break

        # 3) 从峰顶往右找边界，但不能越过 right_limit
        right = peak
        while right < right_limit:
            if data[right] > data[right + 1]:
                right += step
            else:
                if (data[right + 1] - data[right]) > slope_threshold:
                    right += step
                else:
                    break
            if right >= right_limit:
                right = right_limit
                break

        if left > right:
            left, right = right, left

        # 4) 基线估算：只在 [left - extension, right + extension] 内部找最小值/均值
        left_integ = max(0, left - extension)
        right_integ = min(n - 1, right + extension)

        # 这里可以用“局部波谷”逻辑：
        #   baseline_left = np.min(data[left_integ: left+1])
        #   baseline_right = np.min(data[right: right_integ+1])
        # baseline = (baseline_left + baseline_right) / 2

        base_region_size = 15  # 可自行调整
        baseline_left_vals = data[left_integ : left_integ + base_region_size]
        baseline_right_vals = data[right_integ - base_region_size + 1 : right_integ + 1]

        baseline_left = np.mean(baseline_left_vals) if len(baseline_left_vals) else data[left]
        baseline_right = np.mean(baseline_right_vals) if len(baseline_right_vals) else data[right]
        baseline = min(baseline_left, baseline_right)

        # 5) 计算积分
        gross_area = np.trapz(data[left_integ : right_integ + 1], dx=1)
        baseline_area = baseline * (right_integ - left_integ)
        net_area = gross_area - baseline_area

        peak_params.append({
            'peak_index': peak,
            'left_index': left_integ,
            'right_index': right_integ,
            'baseline': baseline,
            'gross_area': gross_area,
            'net_area': net_area
        })

    return peak_params


#用于获取上传姓名和年龄用到的数据
def get_user_by_id(user_id):
    """
    根据用户ID查询用户信息，返回 (id, username, email, age)
    """
    try:
        conn = sqlite3.connect(DATABASE)
        cursor = conn.cursor()
        cursor.execute("SELECT id, username, email, age FROM user_table WHERE id = ?", (user_id,))
        row = cursor.fetchone()
        conn.close()
        return row
    except Exception as e:
        print(f"Error fetching user by id: {e}")
        return None


def custom_find_peaks(data, prominence=50, min_distance=20):
    data = np.asarray(data, dtype=np.float32)
    if data.ndim != 1 or len(data) < 3:
        return np.array([], dtype=int)

    peaks = []
    i = 1
    while i < len(data) - 1:
        # 检查局部最大值
        if data[i] > data[i - 1] and data[i] > data[i + 1]:
            left_edge = i
            right_edge = i
            # 检查是否存在平顶
            while right_edge < len(data) - 1 and data[right_edge] == data[right_edge + 1]:
                right_edge += 1

            # 计算一个简单的baseline（用左右邻点值）
            baseline = max(data[left_edge - 1],
                           data[right_edge + 1] if right_edge < len(data) - 1 else data[right_edge])
            peak_val = data[i]
            if (peak_val - baseline) >= prominence:
                # 用平顶区间的中点作为峰值索引
                peak_index = (left_edge + right_edge) // 2
                peaks.append(peak_index)
            i = right_edge + 1  # 跳过平顶区间
        else:
            i += 1

    # 合并过近的峰
    if min_distance > 0 and len(peaks) > 1:
        # 按峰值高度排序，保留最高的
        peak_heights = [(p, data[p]) for p in peaks]
        peak_heights.sort(key=lambda x: x[1], reverse=True)
        final_peaks = []
        for p, h in peak_heights:
            if all(abs(p - fp) >= min_distance for fp in final_peaks):
                final_peaks.append(p)
        final_peaks.sort()
        peaks = final_peaks

    return np.array(peaks, dtype=int)

#平滑计算
def smooth_data(data, window_size=9):
    """
    简单滑动平均平滑，window_size建议是奇数
    返回与原长度相同的数组（前端和后端的值用padding处理）
    """
    if window_size < 3:
        return data

    half_w = window_size // 2
    smoothed = np.copy(data).astype(float)

    for i in range(len(data)):
        start = max(0, i - half_w)
        end = min(len(data), i + half_w + 1)
        smoothed[i] = np.mean(data[start:end])
    return smoothed

if __name__ == "__main__":
    create_app_database()  # 创建所有表（包括 detection_results）
    app = QApplication(sys.argv)
    window = Welcome1Window()  # 创建欢迎窗口
    window.show()  # 显示欢迎窗口
    sys.exit(app.exec_())  # 启动应用
