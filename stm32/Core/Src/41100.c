
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
������:mygpioinit(void)
����:��ʼ���˿�
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
������:senddata()
����:��������
*******************************************************/

void senddata(u8 gain)
{
    CS_0;
    writebyte(0x11);   //��������,����datasheet5.2������д�����ݼĴ�����������
    writebyte(gain);   //��������,��Χ��0x00��0xff
    CS_1;
}

/*******************************************************
������:writebyte(u8 data)
����:ģ��spi
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
        if (data & 0x80)  //��data��0x80������Ԥ��,Ŀ����ȡ��date��������λ��ֵ,�����λ��1,��ôMOSI�ͻ��ɸߵ�ƽ,����ͻᱣ�ֵ͵�ƽ
        {
            MOSI_1;
        }
        SCK_0;
        SCK_1;
        data <<= 1 ;   //��date����,����ѭ�����бȽ�
    }
}
