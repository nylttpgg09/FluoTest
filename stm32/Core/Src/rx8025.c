
#include "main.h"
#include "i2c.h"
#include "rx8025.h"
#define ENABLE_INT() __set_PRIMASK(0) // ʹ��ȫ���ж� /
#define DISABLE_INT() __set_PRIMASK(1) // ��ֹȫ���ж� */
#define RX8025T_EXT_REG  0x0D
#define RX8025T_SEC_REG  0x00
u8 rx8025_init(void)//return 1 fail.
{
    uint8_t val[3] = {0x00, 0x00, 0x60}; //0x0D��0x0E��0x0F�������Ĵ�����ֵ������ʱ�����Ϊ���롱���£��ر��������ӣ��²�ʱ��Ϊ2�룬��ʱ������жϣ��ر������жϡ�

    //iic_init();  //iic�ӿڳ�ʼ��

    if (rx8025_write_data1(RX8025T_EXT_REG << 4, val, 3))
        return 1;
    return 0;
}
/*******************************************************************************
* ������: u8 rx8025_write_data(u8 addr, u8 *buf, u8 len)
* ����  : дRX8025T�Ĵ���
* ����  : addr�Ĵ�����ַ��*bufд������ݣ�lenд��ĳ���
* ����ֵ: 1=����ʧ�ܣ�0=�����ɹ�
*******************************************************************************/
u8 rx8025_write_data(u8 addr, u8 *buf, u8 len)
{
    uint8_t i, res = 0;
    __disable_irq();
    DISABLE_INT();
    I2CStart();
    I2CSendByte(0x64); //д����ָ��
    if (I2CWaitAck())  //���ACK�ź�
    {
        I2CStop();   //����IICֹͣ�ź�
        res = 1;
        goto err;
    }
    I2CSendByte(addr); //д�Ĵ����洢��ַ
    if (I2CWaitAck())
    {
        I2CStop();
        res = 2;
        goto err;
    }
    for (i = 0; i < len; i++) //����д
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
    I2CSendByte(0x64); //д����ָ��
  if(I2CWaitAck())   //���ACK�ź�
  {
      I2CStop();   //����IICֹͣ�ź�
      res=1;
      goto err;
  }
    I2CSendByte(addr); //д�Ĵ����洢��ַ
  if(I2CWaitAck())
  {
      I2CStop();
      res=2;
      goto err;
  }
    for (i = 0; i < len; i++) //����д
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
* ������: u8 rx8025_read_data(u8 addr,u8 *buf,u8 len)
* ����  : ��RX8025T�Ĵ���
* ����  : addr�Ĵ�����ַ��*buf�洢λ�ã�len��ȡ�ĳ���
* ����ֵ: 1=����ʧ�ܣ�0=�����ɹ�
jjw  �Ĵ���Ӧ����4λ���Ӵ���ģʽ����������û�С����ԼĴ���RX8025T_SEC_REGӦ������4λ
*******************************************************************************/
u8 rx8025_read_data(u8 addr, u8 *buf, u8 len)
{
    u8 i;
    I2CStart();
    I2CSendByte(0x64);  //д����ָ��
    if (I2CWaitAck())
    {
        I2CStop();
        return 1;
    }
    I2CSendByte(addr);  //���ͼĴ�����ַ
    if (I2CWaitAck())
    {
        I2CStop();
        return 1;
    }
    I2CStart();          //Sr������RESTART
    I2CSendByte(0x65);  //������ָ��
    if (I2CWaitAck())
    {
        I2CStop();
        return 1;
    }
    for (i = 0; i < len; i++) //������
    {
        buf[i] = I2CReceiveByte(); //��ȡ���ݲ�����ACK�ź�
        I2CSendAck();//jjw
    }
    //  iic_nack();         //����һ����1��
    I2CSendNotAck();//jjw
    I2CStop();
    return 0;
}

/*******************************************************************************
* ������: u8 get_rx8025_time(TIME* t)
* ����  : ��RX8025T��ȡʱ��
* ����  : �洢ʱ��Ľṹ��
* ����ֵ: 0�ɹ���1ʧ�ܡ�
*******************************************************************************/
u8 get_rx8025_time(TIME *t)
{
    u8 rtc_str[7];

    if (rx8025_read_data(RX8025T_SEC_REG, rtc_str, 7)) //��ȡ������ʱ��
        return 1;  //��ȡ����

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
* ������: u8 set_rx8025_time(TIME* t)
* ����  : ����RX8025Tʱ��
* ����  : �洢ʱ��Ľṹ��
* ����ֵ: 0�ɹ���1ʧ�ܡ�
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

    if (rx8025_write_data(RX8025T_SEC_REG, rtc_str, 7)) //д��������ʱ��
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

    if (rx8025_write_data1(RX8025T_SEC_REG, rtc_str, 7)) //д��������ʱ�� 1-ʧ��
        return 1;  //1-ʧ��
    else
        return 0;
}