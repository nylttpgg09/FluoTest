import serial
import serial.tools.list_ports
import os
import ctypes
from ctypes import *
import time


# 获取所有串口设备实例。
# 如果没找到串口设备，则输出：“无串口设备。”
# 如果找到串口设备，则依次输出每个设备对应的串口号和描述信息。
ports_list = list(serial.tools.list_ports.comports())
if len(ports_list) <= 0:
    print("无串口设备。")
else:
    print("可用的串口设备如下：")
    for comport in ports_list:
        print(list(comport)[0], list(comport)[1])
# 方式1：调用函数接口打开串口时传入配置参数
 
ser = serial.Serial("/dev/ttyCH343USB0", 115200,timeout=2)    # 打开COM，将波特率配置为115200，其余参数使用默认值
if ser.isOpen():                        # 判断串口是否成功打开
    print("打开串口成功。")
    print(ser.name)    # 输出串口号
else:
    print("打开串口失败。")

wbuf = ctypes.create_string_buffer(7)
#\x5A\xA5\x01\x02\x01\x00\x03 启动检测
wbuf[0]=0x5a
wbuf[1]=0xa5
wbuf[2]=0x01
wbuf[3]=0x02
wbuf[4]=0x01
wbuf[5]=0x00
wbuf[6]=0x03
write_len = ser.write(wbuf)

print("串口发出{}个字节。暂停20秒，读取数据".format(write_len))
# 暂停10秒
time.sleep(20)
wbuf = ctypes.create_string_buffer(8)
#x5A\xA5\x01\x03\x02\x03\xe8\xf0  读数据
wbuf[0]=0x5a
wbuf[1]=0xa5
wbuf[2]=0x01
wbuf[3]=0x03
wbuf[4]=0x02
wbuf[5]=0x03
wbuf[6]=0xe8
wbuf[7]=0xf0
write_len = ser.write(wbuf)

print("串口发出{}个字节。".format(write_len))


com_input = ser.read(5000)
if com_input:
    # 将字节流转换为十六进制格式输出
    hex_output = ' '.join(f'{byte:02x}' for byte in com_input)
    print("读取数据成功：", hex_output)

    # 处理和打印字符串中的可打印字符
    printable_data = ''.join([chr(byte) if 32 <= byte <= 126 else '.' for byte in com_input])
    print("可打印字符：", printable_data)

ser.close()
