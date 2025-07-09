基于STM32F407VGT6+华为IOT+全志v3s自制开发板的智能荧光孕激素检测分析仪 ，用于检测荧光试剂。
基于STM32F407VGT6+华为IOT的智能荧光孕激素检测分析仪，是一款便携式孕激素定量检测仪，实现对孕激素含量的定量检测。
系统采用光电检测法、物联网技术、传感器技术和实时数据处理，实现智能化的对女性尿液中孕激素含量的定量检测，可通过华为云平台，手机APP等实时反馈。
环境配置：
嵌入式linux系统（linux4.13 + pyqt5）
pip install qtpy

pip install paho.mqtt 

pip install sqlite

配置wifi文件
mkdir  /lib/firmware/rtlwifi  -p
cp  rtl8188eufw.bin  /lib/firmware/rtlwifi 
cp wpa_supplicant.conf /etc/wpa_supplicant.conf
cp r8188eu.ko /root
cp wifi.sh /root
vi /etc/profile

使用说明：
1.运行flow.py后，先会自动连接mqtt华为云平台
出现这个说明连接成功：

然后进入系统启动界面

1.	加载完进入主界面：

批量测试（测试数据管理界面）：
 
点击计算后获取到数据结果，这是pycharm和串口会接受到的内容

计算界面：点击计算获取数据内容8.5-10之间即在正常范围。

用户选择界面：用户列表选择用户，（可以再项目里面添加用户），点击修改用户跳转至计算界面，即可绑定用户信息。

用户管理界面：点击insert user即可插入用户，生成对应的id，点击load可加载目前已有的用户
