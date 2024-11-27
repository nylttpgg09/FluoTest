import matplotlib.pyplot as plt
import numpy as np

# 读取数据
data_values = []

# 打开文件并读取
with open('data.txt', 'r') as file:
    # 读取整个文件内容，并拆分为列表
    hex_data = file.read().split()

    # 忽略前六个帧头
    hex_data = hex_data[6:]

    # 将每两个十六进制数转换为16位无符号数据
    for i in range(0, len(hex_data), 2):
        # 获取每一对十六进制数
        hex_pair = hex_data[i:i + 2]

        # 检查是否有两个十六进制数来避免超出范围
        if len(hex_pair) == 2:
            # 将低位字节在前，高位字节在后的方式转换为16位整数
            low_byte = int(hex_pair[0], 16)  # 低位字节
            high_byte = int(hex_pair[1], 16)  # 高位字节

            # 合并为一个16位整数（小端编码）
            value = low_byte + (high_byte << 8)

            # 将值添加到列表中，已经是十进制
            data_values.append(value)

# 假设数据是一个列表，其中包含要绘制的Y值
# 设置X轴范围为0到1000（如果数据量少于1000，可以重复数据或限制）
x_values = list(range(0, min(1000, len(data_values))))

# 如果数据少于1000个，可以重复数据直到1000个点（可选）
if len(data_values) < 1000:
    data_values = data_values * (1000 // len(data_values)) + data_values[:(1000 % len(data_values))]

# 使用滑动窗口进行平滑（例如窗口大小为5）
window_size = 5
smoothed_data = np.convolve(data_values, np.ones(window_size) / window_size, mode='valid')

# 绘制平滑后的曲线
plt.plot(x_values[:len(smoothed_data)], smoothed_data, label='Data')

# 添加标题和标签
plt.title('data')
plt.xlabel('X Values (0-1000)')
plt.ylabel('Data Values (0-65535)')

# 显示图例
plt.legend()

# 展示图形
plt.show()
