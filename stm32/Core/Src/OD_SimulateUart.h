#ifndef __OD_SIMULATEUART_H__
#define __OD_SIMULATEUART_H__

#include <stdint.h>
#include "main.h"
#include "OD_CircularQueue.h"

#define OD_SUART_BAUD         9600
#define OD_SUART_WORD_LEN     UART_DATA_BITS_8
#define OD_SUART_STOPBIT      UART_STOP_BITS_1
#define OD_SUART_PARITY       UART_PAIRTY_NO
#define OD_SUART_BUFF_SIZE    128

typedef unsigned long   uint32;
typedef unsigned short  uint16;
typedef unsigned char   uint8;
/*ͨ�Ŵ����״̬*/
typedef struct State
{
    uint8 BitNum;         /*�ܹ�λ�ĸ���*/
    uint8 BitCount;       /*��ǰ����λ�ļ��������յ�����λ������ֹͣλ����*/
    uint8 HalfBitCount;   /*1/2bit����*/
    uint8 ErrorFlag;      /*����ı��*/
    uint16 Data;          /*��ǰ���������*/
} ComState_STR;

/*����������Ϣ*/
typedef struct config
{
    uint8 DataBitNum;   /*��������λ�ĸ�����*/
    uint8 PairtyState;  /*����У���״̬����żУ�飬��У��*/
    uint8 StopBitNum;   /*����ֹͣλ�ĸ�����1λ��2λ*/
    uint8 TxEnable;     /*����ʹ��*/
    uint8 RxEnable;     /*����ʹ��*/
    uint32 BaudRate;    /*���õĲ�����*/
} UartConfig_STR;

/*ģ�⴮�ڵ�������Ϣ*/
typedef struct uart
{
    ComState_STR TX;
    ComState_STR RX;
    UartConfig_STR Config;   //����
    GPIO_TypeDef *Tx_GPIOx;       //TX�˿�
    uint16_t     Tx_GPIO_Pin;     //TX����
    GPIO_TypeDef *Rx_GPIOx;       //RX�˿�
    uint16_t     Rx_GPIO_Pin;     //RX����
    IRQn_Type IRQn;								//�ж�
    CirQueue_Str SUartQueue;    //����
    uint8 SUartQueueBuff[OD_SUART_BUFF_SIZE];  //������
} SimulateUart_STR;

/*����ģ���ʹ��*/
#define UART_ENABLE           0x01    /*ʹ�ܷ��ͽ���*/
#define UART_DISABLE          0x00    /*��ֹ���ͽ���*/

/*���ڴ�����*/
#define UART_ERROR_PAIRTY     0x01    /*У�����*/
#define UART_ERROR_STOP       0x02    /*ֹͣλ���մ���*/
#define UART_ERROR_TIMEOUT    0x04    /*���ճ�ʱ*/

/*����λ��������һ��ΪĬ��*/
#define UART_DATA_BITS_8      0x08    /*8λ����*/
#define UART_DATA_BITS_7      0x07    /*7λ����*/

/*��żУ�飬��һ��ΪĬ��*/
#define UART_PAIRTY_NO        0x00    /*����żУ��*/
#define UART_PAIRTY_ODD       0x01    /*��У��*/
#define UART_PAIRTY_EVEN      0x02    /*żУ��*/

/*ֹͣλ��������һ��ΪĬ��*/
#define UART_STOP_BITS_1      0x01    /*1λֹͣλ*/
#define UART_STOP_BITS_2      0x02    /*2λֹͣλ*/




extern uint8 OD_OneBitNum(uint16 dat, uint8 num);
void OD_SimulateUartInit(SimulateUart_STR *SUart);
void OD_SimulateUartMain(SimulateUart_STR *SUart);
void OD_SimulateUartDetectStart(SimulateUart_STR *SUart);
extern void OD_SimulateUartWriteByte(SimulateUart_STR *SUart,uint8 dat);
void OD_SimulateUartWriteData(SimulateUart_STR *SUart,uint8 *wdata, uint16 len);
uint16 OD_SimulateUartRead(SimulateUart_STR *SUart,uint8 *rdata);
void SimUart_Init(void);
extern SimulateUart_STR SUart4;
extern SimulateUart_STR SUart5;
extern SimulateUart_STR SUart6;
#endif /*__OD_SIMULATEUART_H__*/
