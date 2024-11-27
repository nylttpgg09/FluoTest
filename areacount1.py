import matplotlib.pyplot as plt
import numpy as np
from scipy.signal import find_peaks

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

# 假设 data_values 是从上面生成的曲线数据
data_values = np.array(data_values)  # 确保 data_values 是 numpy 数组类型


def calculate_peak_area_base_line(data):
    # 通过 find_peaks 查找波峰，并设置参数来避免过多波峰
    peaks, properties = find_peaks(data, prominence=100, width=5)

    # 存储每个波峰的面积
    peak_areas = []

    # 对每个波峰进行处理
    for peak in peaks:
        # 扩大范围检测波峰的左右边界
        left = peak
        right = peak

        # 向左扩展，直到找到一个低于当前波峰的点
        while left > 0 and data[left] > data[left - 1]:
            left -= 10
        # 向右扩展，直到找到一个低于当前波峰的点
        while right < len(data) - 1 and data[right] > data[right + 1]:
            right += 10

        # 增加检测范围：以防基线检测不准确
        # 通过扩大左右边界来捕获波峰两侧的最低点
        left_extended = max(0, left - 100)  # 扩展左边界
        right_extended = min(len(data) - 1, right + 100)  # 扩展右边界

        # 获取波峰两侧的最低点（基线），并使用平滑方法来选择基线
        baseline_left = np.mean(data[left_extended:peak + 1])  # 使用平均值
        baseline_right = np.mean(data[peak:right_extended + 1])  # 使用平均值

        # 计算基线（左侧和右侧的最低点）
        baseline = min(baseline_left, baseline_right)

        # 对于平坦波峰，自定义基线
        if (data[peak] - baseline) < 50:  # 判断顶部平坦的波峰（可以根据实际数据调整阈值）
            # 使用一个平滑方法替代简单的最小值基线，可能更适合平坦顶部波峰
            baseline = np.mean([baseline_left, baseline_right])

        # 使用线本底法计算波峰面积
        peak_area = np.trapz(data[left:right + 1] - baseline, dx=1)  # 减去基线并使用梯形法计算面积

        peak_areas.append(peak_area)

    return peaks, peak_areas


# 绘制原始曲线
plt.plot(range(len(data_values)), data_values, label='Original Data')

# 计算波峰及其面积
peaks, peak_areas = calculate_peak_area_base_line(data_values)

# 绘制波峰标记
plt.plot(peaks, [data_values[i] for i in peaks], "rx", label="Peaks")

# 显示图例
plt.legend()

# 添加标题和标签
plt.title('Peak Area')
plt.xlabel('X Values')
plt.ylabel('Data Values')

# 显示图形
plt.show()

# 输出每个波峰的面积
for i, area in enumerate(peak_areas):
    print(f"Peak {i + 1}: Area = {area}")

# 选择两个波峰并计算面积比值（选择合适的波峰，需要自行调整）
if len(peak_areas) >= 2:
    peak_area_ratio = peak_areas[5] / peak_areas[8]
    print(f"The ratio of the first peak area to the second peak area is: {peak_area_ratio}")
else:
    print("Not enough peaks detected for ratio calculation.")
