
#include "main.h"

/*
CS-PA4
CLK-PA5
SI-PA7

*/


#define SCK_1 HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET)
#define SCK_0 HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET)
#define MOSI_1 HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, GPIO_PIN_SET)
#define MOSI_0 HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, GPIO_PIN_RESET)
#define CS_1 HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET)
#define CS_0 HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET)

#define u8 uint8_t

void writebyte(u8 data);
void init_41100(u8 gain);
void senddata(u8 gain);

/*******************************************************
函数名:mygpioinit(void)
功能:初始化端口
*******************************************************/
void init_41100(u8 gain)
{

    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* GPIO Ports Clock Enable */
    __HAL_RCC_GPIOA_CLK_ENABLE();

    GPIO_InitStruct.Pin = GPIO_PIN_4;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_5;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_7;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
	
	senddata(gain);
}


/*******************************************************
函数名:senddata()
功能:发送数据
*******************************************************/

void senddata(u8 gain)
{
    CS_0;
    writebyte(0x11);   //发送命令,参照datasheet5.2将数据写入数据寄存器进行配置
    writebyte(gain);   //发送数据,范围是0x00到0xff
    CS_1;
}

/*******************************************************
函数名:writebyte(u8 data)
功能:模拟spi
*******************************************************/
void writebyte(u8 data)
{
    u8 i;
    SCK_0;
    MOSI_0;
    for (i = 0; i < 8; i++)
    {
        MOSI_0;
        SCK_0;
        if (data & 0x80)  //将data与0x80进行与预算,目的是取得date二进制首位数值,如果首位是1,那么MOSI就会变成高电平,否则就会保持低电平
        {
            MOSI_1;
        }
        SCK_0;
        SCK_1;
        data <<= 1 ;   //将date左移,继续循环进行比较
    }
}
