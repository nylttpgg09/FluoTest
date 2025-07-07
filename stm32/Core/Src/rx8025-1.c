#include "main.h"

#define SCL_1    	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_6,GPIO_PIN_SET);
#define SDA_1		HAL_GPIO_WritePin(GPIOB,GPIO_PIN_7,GPIO_PIN_SET);
#define SCL_0		HAL_GPIO_WritePin(GPIOB,GPIO_PIN_6,GPIO_PIN_RESET);
#define SDA_0		HAL_GPIO_WritePin(GPIOB,GPIO_PIN_7,GPIO_PIN_RESET);
#define READ_SDA 	HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_7) == GPIO_PIN_SET

#define u8 uint8_t
#define u16 uint16_t

void SDA_OUT()
{
    GPIO_InitTypeDef GPIO_InitStructure = {0};

    GPIO_InitStructure.Pin = GPIO_PIN_7;
    GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_OD;
    GPIO_InitStructure.Pull = GPIO_NOPULL;
    GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);
}
void SDA_IN()
{
    GPIO_InitTypeDef GPIO_InitStructure = {0};

    GPIO_InitStructure.Pin = GPIO_PIN_7;
    GPIO_InitStructure.Mode = GPIO_MODE_INPUT;
    GPIO_InitStructure.Pull = GPIO_PULLUP;
    GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);
}

void Delayus(u16 us)
{
    u16 i;
    for (i = 0; i < us; i++)
        __nop();
}


void IIC_Init(void)
{
//    GPIO_InitTypeDef GPIO_InitStructure;
//    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

//    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
//    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP ;   //推挽输出
//    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
//    GPIO_Init(GPIOB, &GPIO_InitStructure);
////  GPIO_SetBits(GPIOB,GPIO_Pin_6|GPIO_Pin_7);  //PB6,PB7 输出高
}

void IIC_Start(void)
{
    SDA_OUT();
    SCL_1
    SDA_1
    Delayus(10);
    SDA_0
    Delayus(10);
    SCL_0
    Delayus(10);



}

void IIC_Stop(void)
{
    u8 i;
    SDA_OUT();//sda线输出
    SDA_0
    Delayus(10);
    SCL_1
    Delayus(10);
    SDA_1
    for (i = 0; i < 5; i++)
    {
        Delayus(10);
    }
}

u8 IIC_Wait_Ack(void)
{
    u8 ucErrTime = 0;
    SDA_IN();      //SDA设置为输入
    Delayus(10);
    Delayus(10);
    while (READ_SDA)
    {
        ucErrTime++;
        if (ucErrTime > 250)
        {
            IIC_Stop();
            return 1;
        }

    }
    SCL_0//时钟输出0
    return 0;
}

void IIC_Ack(void)
{

    SDA_OUT();
    SDA_0;
    Delayus(10);
    SCL_1
    Delayus(10);
    SCL_0
    Delayus(10);
}

void IIC_NAck(void)
{
    SDA_OUT();
    SDA_1;
    Delayus(10);
    SCL_1
    Delayus(10);
    SCL_0
    Delayus(10);
}

void IIC_Send_Byte(u8 txd)
{
    u8 i;
    SDA_OUT();
    for (i = 0; i < 8; i++)
    {
        if (txd & 0x80) SDA_1
            else SDA_0
                txd <<= 1;
        Delayus(10);
        SCL_1
        Delayus(10);
        SCL_0
    }
    Delayus(10);
    SCL_1
    Delayus(10);
    SCL_0
}

u8 IIC_Read_Byte(unsigned char ack)
{
    u8 i, receive = 0;
    SDA_IN();//SDA设置为输入

    for (i = 0; i < 8; i++)
    {
        SCL_1
        Delayus(10);
        receive <<= 1;
        if (READ_SDA) receive |= 0x01;

        SCL_0
        Delayus(10);
    }
    SDA_OUT();
    return receive;

}


void R8025AC_Read(u8 sadd, u8 *buf, u8 len)  //JJW 寄存器地址，左移4位加传输模式 这是AC型号，与T不同
{
    u8 i;
    IIC_Start();
    IIC_Send_Byte(0x64);
    IIC_Send_Byte(sadd);
    IIC_Start();
    IIC_Send_Byte(0x65);
    for (i = 0; i < len - 1; i++)
    {
        buf[i] = IIC_Read_Byte(1);
        IIC_Ack();
    }
    buf[i] = IIC_Read_Byte(0);
    IIC_NAck();
    IIC_Stop();
}

void R8025AC_Write(u8 sadd, u8 *buf, u8 len)
{
    u8 i;

    IIC_Start();
    IIC_Send_Byte(0X64);
    IIC_Send_Byte(sadd );

    for (i = 0; i < len; i++)
    {
        IIC_Send_Byte(buf[i]);
    }
    IIC_Stop();
}

