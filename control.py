import serial
import serial.tools.list_ports
import os
import ctypes
from ctypes import *
import time
def get_serial_data():
    # 获取所有串口设备实例。
    ports_list = list(serial.tools.list_ports.comports())
    if len(ports_list) <= 0:
        print("无串口设备。")
        return None
    else:
        print("可用的串口设备如下：")
        for comport in ports_list:
            print(list(comport)[0], list(comport)[1])
    
    # 通过指定串口打开
    ser = serial.Serial("/dev/ttyCH343USB0", 115200, timeout=2)  # 根据你的实际情况修改COM端口号
    if ser.isOpen():
        print("打开串口成功。")
        print(ser.name)  # 输出串口号
    else:
        print("打开串口失败。")
        return None

    # 启动检测命令
    wbuf = ctypes.create_string_buffer(7)
    wbuf[0] = 0x5a
    wbuf[1] = 0xa5
    wbuf[2] = 0x01
    wbuf[3] = 0x02
    wbuf[4] = 0x01
    wbuf[5] = 0x00
    wbuf[6] = 0x03
    write_len = ser.write(wbuf)

    print("串口发出{}个字节。暂停20秒，读取数据".format(write_len))
    time.sleep(20)

    # 读数据
    wbuf = ctypes.create_string_buffer(8)
    wbuf[0] = 0x5a
    wbuf[1] = 0xa5
    wbuf[2] = 0x01
    wbuf[3] = 0x03
    wbuf[4] = 0x02
    wbuf[5] = 0x03
    wbuf[6] = 0xe8
    wbuf[7] = 0xf0
    write_len = ser.write(wbuf)

    print("串口发出{}个字节。".format(write_len))

    # 读取串口数据
    com_input = ser.read(5000)

    if com_input:
        # 将字节流转换为十六进制格式输出
        hex_output = ' '.join(f'{byte:02x}' for byte in com_input)
        print("读取数据成功：", hex_output)

        # 处理和打印字符串中的可打印字符
        printable_data = ''.join([chr(byte) if 32 <= byte <= 126 else '.' for byte in com_input])
        print("可打印字符：", printable_data)

        return com_input
    else:
        print("未读取到数据。")
        return None
    ser.close()