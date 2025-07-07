
#include "main.h"
#include "i2c.h"
#include "rx8025.h"
#define ENABLE_INT() __set_PRIMASK(0) // 使能全局中断 /
#define DISABLE_INT() __set_PRIMASK(1) // 禁止全局中断 */
#define RX8025T_EXT_REG  0x0D
#define RX8025T_SEC_REG  0x00
u8 rx8025_init(void)//return 1 fail.
{
    uint8_t val[3] = {0x00, 0x00, 0x60}; //0x0D、0x0E、0x0F、三个寄存器的值，设置时间更新为“秒”更新，关闭所有闹钟，温补时间为2秒，打开时间更新中断，关闭其他中断。

    //iic_init();  //iic接口初始化

    if (rx8025_write_data1(RX8025T_EXT_REG << 4, val, 3))
        return 1;
    return 0;
}
/*******************************************************************************
* 函数名: u8 rx8025_write_data(u8 addr, u8 *buf, u8 len)
* 描述  : 写RX8025T寄存器
* 参数  : addr寄存器地址，*buf写入的数据，len写入的长度
* 返回值: 1=操作失败，0=操作成功
*******************************************************************************/
u8 rx8025_write_data(u8 addr, u8 *buf, u8 len)
{
    uint8_t i, res = 0;
    __disable_irq();
    DISABLE_INT();
    I2CStart();
    I2CSendByte(0x64); //写操作指令
    if (I2CWaitAck())  //检测ACK信号
    {
        I2CStop();   //发送IIC停止信号
        res = 1;
        goto err;
    }
    I2CSendByte(addr); //写寄存器存储地址
    if (I2CWaitAck())
    {
        I2CStop();
        res = 2;
        goto err;
    }
    for (i = 0; i < len; i++) //连续写
    {
        I2CSendByte(buf[i]);
        if (I2CWaitAck())
        {
            I2CStop();
            res = 3;
            goto err;
        }
    }
    I2CStop();
    __enable_irq();
    return 0;
err:
    __enable_irq();
    
    return res;
}
u8 rx8025_write_data1(u8 addr, u8 *buf, u8 len)
{
    uint8_t i, res = 0;
	DISABLE_INT();
    I2CStart();
    I2CSendByte(0x64); //写操作指令
  if(I2CWaitAck())   //检测ACK信号
  {
      I2CStop();   //发送IIC停止信号
      res=1;
      goto err;
  }
    I2CSendByte(addr); //写寄存器存储地址
  if(I2CWaitAck())
  {
      I2CStop();
      res=2;
      goto err;
  }
    for (i = 0; i < len; i++) //连续写
    {
        I2CSendByte(buf[i]);
      if(I2CWaitAck())
      {
          I2CStop();
          res=3;
          goto err;
      }
    }
    I2CStop();
	ENABLE_INT();
    return 0;
	err:ENABLE_INT();
	return res;
}

/*******************************************************************************
* 函数名: u8 rx8025_read_data(u8 addr,u8 *buf,u8 len)
* 描述  : 读RX8025T寄存器
* 参数  : addr寄存器地址，*buf存储位置，len读取的长度
* 返回值: 1=操作失败，0=操作成功
jjw  寄存器应左移4位，加传输模式，但代码中没有。所以寄存器RX8025T_SEC_REG应做左移4位
*******************************************************************************/
u8 rx8025_read_data(u8 addr, u8 *buf, u8 len)
{
    u8 i;
    I2CStart();
    I2CSendByte(0x64);  //写操作指令
    if (I2CWaitAck())
    {
        I2CStop();
        return 1;
    }
    I2CSendByte(addr);  //发送寄存器地址
    if (I2CWaitAck())
    {
        I2CStop();
        return 1;
    }
    I2CStart();          //Sr条件，RESTART
    I2CSendByte(0x65);  //读操作指令
    if (I2CWaitAck())
    {
        I2CStop();
        return 1;
    }
    for (i = 0; i < len; i++) //连续读
    {
        buf[i] = I2CReceiveByte(); //读取数据并发送ACK信号
        I2CSendAck();//jjw
    }
    //  iic_nack();         //发送一个‘1’
    I2CSendNotAck();//jjw
    I2CStop();
    return 0;
}

/*******************************************************************************
* 函数名: u8 get_rx8025_time(TIME* t)
* 描述  : 从RX8025T获取时间
* 参数  : 存储时间的结构体
* 返回值: 0成功，1失败。
*******************************************************************************/
u8 get_rx8025_time(TIME *t)
{
    u8 rtc_str[7];

    if (rx8025_read_data(RX8025T_SEC_REG, rtc_str, 7)) //获取日期与时间
        return 1;  //读取出错

    t->second = ((rtc_str[0] >> 4) * 10) + (rtc_str[0] & 0x0f);
    t->minute = ((rtc_str[1] >> 4) * 10) + (rtc_str[1] & 0x0f);
    t->hour   = ((rtc_str[2] >> 4) * 10) + (rtc_str[2] & 0x0f);
    t->week     = rtc_str[3];
    t->day    = ((rtc_str[4] >> 4) * 10) + (rtc_str[4] & 0x0f);
    t->month  = ((rtc_str[5] >> 4) * 10) + (rtc_str[5] & 0x0f);
    t->year   = ((rtc_str[6] >> 4) * 10) + (rtc_str[6] & 0x0f);
    t->year += 2000;
    return 0;

}
/*******************************************************************************
* 函数名: u8 set_rx8025_time(TIME* t)
* 描述  : 设置RX8025T时间
* 参数  : 存储时间的结构体
* 返回值: 0成功，1失败。
*******************************************************************************/
u8 set_rx8025_time(u16 year, u8 month, u8 day, u8 week, u8 hour, u8 minute, u8 second)
{
    u8 rtc_str[7];
    year -= 2000;
    rtc_str[0] = ((second / 10) << 4) | (second % 10);
    rtc_str[1] = ((minute / 10) << 4) | (minute % 10);
    rtc_str[2] = ((hour / 10) << 4) | (hour % 10);
    rtc_str[3] = week;
    rtc_str[4] = ((day / 10) << 4) | (day % 10);
    rtc_str[5] = ((month / 10) << 4) | (month % 10);
    rtc_str[6] = ((year / 10) << 4) | (year % 10);

    if (rx8025_write_data(RX8025T_SEC_REG, rtc_str, 7)) //写入日期与时间
        return 1;
    else
        return 0;
}
u8 set_rx8025_time_t(TIME t)
{
    u8 rtc_str[7];
    t.year -= 2000;
    rtc_str[0] = ((t.second / 10) << 4) | (t.second % 10);
    rtc_str[1] = ((t.minute / 10) << 4) | (t.minute % 10);
    rtc_str[2] = ((t.hour / 10) << 4) | (t.hour % 10);
    rtc_str[3] = t.week;
    rtc_str[4] = ((t.day / 10) << 4) | (t.day % 10);
    rtc_str[5] = ((t.month / 10) << 4) | (t.month % 10);
    rtc_str[6] = ((t.year / 10) << 4) | (t.year % 10);

    if (rx8025_write_data1(RX8025T_SEC_REG, rtc_str, 7)) //写入日期与时间 1-失败
        return 1;  //1-失败
    else
        return 0;
}