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
/*通信传输的状态*/
typedef struct State
{
    uint8 BitNum;         /*总共位的个数*/
    uint8 BitCount;       /*当前处理位的计数，接收到数据位个数，停止位个数*/
    uint8 HalfBitCount;   /*1/2bit计数*/
    uint8 ErrorFlag;      /*出错的标记*/
    uint16 Data;          /*当前处理的数据*/
} ComState_STR;

/*串口配置信息*/
typedef struct config
{
    uint8 DataBitNum;   /*配置数据位的个数，*/
    uint8 PairtyState;  /*配置校验的状态，奇偶校验，无校验*/
    uint8 StopBitNum;   /*配置停止位的个数，1位，2位*/
    uint8 TxEnable;     /*发送使能*/
    uint8 RxEnable;     /*接收使能*/
    uint32 BaudRate;    /*配置的波特率*/
} UartConfig_STR;

/*模拟串口的数据信息*/
typedef struct uart
{
    ComState_STR TX;
    ComState_STR RX;
    UartConfig_STR Config;   //配置
    GPIO_TypeDef *Tx_GPIOx;       //TX端口
    uint16_t     Tx_GPIO_Pin;     //TX引脚
    GPIO_TypeDef *Rx_GPIOx;       //RX端口
    uint16_t     Rx_GPIO_Pin;     //RX引脚
    IRQn_Type IRQn;								//中断
    CirQueue_Str SUartQueue;    //队列
    uint8 SUartQueueBuff[OD_SUART_BUFF_SIZE];  //缓冲区
} SimulateUart_STR;

/*串口模块的使能*/
#define UART_ENABLE           0x01    /*使能发送接收*/
#define UART_DISABLE          0x00    /*禁止发送接收*/

/*串口错误标记*/
#define UART_ERROR_PAIRTY     0x01    /*校验错误*/
#define UART_ERROR_STOP       0x02    /*停止位接收错误*/
#define UART_ERROR_TIMEOUT    0x04    /*接收超时*/

/*数据位个数，第一个为默认*/
#define UART_DATA_BITS_8      0x08    /*8位数据*/
#define UART_DATA_BITS_7      0x07    /*7位数据*/

/*奇偶校验，第一个为默认*/
#define UART_PAIRTY_NO        0x00    /*无奇偶校验*/
#define UART_PAIRTY_ODD       0x01    /*奇校验*/
#define UART_PAIRTY_EVEN      0x02    /*偶校验*/

/*停止位个数，第一个为默认*/
#define UART_STOP_BITS_1      0x01    /*1位停止位*/
#define UART_STOP_BITS_2      0x02    /*2位停止位*/




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
