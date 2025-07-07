/*******************************************************************************
* Copyright (C), 2014-2015, www.njodin.com
*
* 文件名: OD_SimulateUart.c
* 内容简述: 模拟串口驱动层
*
* 文件历史:
* 版本号    日期      作者      说明
* v0.1    2014.9.4   candy     创建文件
*
*******************************************************************************/

#include "OD_SimulateUart.h"
#include "main.h"


//volatile CirQueue_Str SUartQueue;
//uint8 SUartQueueBuff[OD_SUART_BUFF_SIZE];
/*******************************************************************************
*  函数名： od_SimulateUartPortConfig
*  输  入:  无
*  输  出:  无
*  功能说明： 串口端口配置
*
*******************************************************************************/

void SimUart_Init(void)
{
    //初始化模拟串口,步骤1-设置模拟串口的引脚和中断,步骤2-调用OD_SimulateUartInit.
//  模拟串口定义为外部全局变量,速率统一为9600
    //修改定时器中断，以及增加3个Exti中断

    SUart4.Tx_GPIOx = GPIOA;
    SUart4.Tx_GPIO_Pin = GPIO_PIN_12;
    SUart4.Rx_GPIOx = GPIOA;
    SUart4.Rx_GPIO_Pin = GPIO_PIN_11;
    SUart4.IRQn = EXTI15_10_IRQn;
    OD_SimulateUartInit(&SUart4);//Uart4  PA12 PA11

    SUart5.Tx_GPIOx = GPIOA;
    SUart5.Tx_GPIO_Pin = GPIO_PIN_1;
    SUart5.Rx_GPIOx = GPIOA;
    SUart5.Rx_GPIO_Pin = GPIO_PIN_6;
    SUart5.IRQn = EXTI9_5_IRQn;
    OD_SimulateUartInit(&SUart5);//Uart5  PA1  PA6

    SUart6.Tx_GPIOx = GPIOA;
    SUart6.Tx_GPIO_Pin = GPIO_PIN_15;
    SUart6.Rx_GPIOx = GPIOB;
    SUart6.Rx_GPIO_Pin = GPIO_PIN_8;
    SUart6.IRQn = EXTI9_5_IRQn;
    OD_SimulateUartInit(&SUart6);//Uart6  PA15  PB8

}
static void od_SimulateUartPortConfig(SimulateUart_STR *SUart)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* GPIO Ports Clock Enable */
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_GPIOE_CLK_ENABLE();
    __HAL_RCC_GPIOF_CLK_ENABLE();
    //初始化模拟串口引脚
    HAL_GPIO_WritePin(SUart->Tx_GPIOx, SUart->Tx_GPIO_Pin, GPIO_PIN_SET);

    GPIO_InitStruct.Pin = SUart->Tx_GPIO_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(SUart->Tx_GPIOx, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = SUart->Rx_GPIO_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(SUart->Rx_GPIOx, &GPIO_InitStruct);

    HAL_NVIC_SetPriority(SUart->IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(SUart->IRQn);

    //初始化队列
    OD_QueueInit((CirQueue_Str *)&(SUart->SUartQueue), SUart->SUartQueueBuff, OD_SUART_BUFF_SIZE);
}


/*******************************************************************************
*  函数名： OD_OneBitNum
*  输  入:  dat：输入的数据
*           num：数据的位数
*  输  出:  1的个数
*  功能说明： 计算数据中1的个数
*
*******************************************************************************/
uint8 OD_OneBitNum(uint16 dat, uint8 num)
{
    register uint8 i;
    uint8 bitNum = 0;
    for (i = 0; i < num; i++)
    {
        bitNum += (dat % 2);        /*获取最低位的值*/
        dat >>= 1;                  /*右移数据*/
    }

    return bitNum;
}

/*******************************************************************************
*  函数名： od_SimulateUartModeConfig
*  输  入:  无
*  输  出:  无
*  功能说明： 串口配置程序
*******************************************************************************/
static void od_SimulateUartModeConfig(SimulateUart_STR *SUart)
{
//初始化 SUart结构体

    SUart->Config.BaudRate = OD_SUART_BAUD;
    SUart->Config.DataBitNum = OD_SUART_WORD_LEN;
    SUart->Config.PairtyState = OD_SUART_PARITY;
    SUart->Config.StopBitNum = OD_SUART_STOPBIT;
    SUart->Config.TxEnable = UART_ENABLE;
    SUart->Config.RxEnable = UART_ENABLE;

    SUart->TX.BitCount = 0;                    /*发送的数据计数清零*/
    SUart->TX.HalfBitCount = 0;                /*一位数据传输时间的一半计数清零*/
    SUart->TX.Data = 0x0000;                   /*发送数据清零*/
    SUart->Config.TxEnable = UART_DISABLE;     /*发送使能关闭*/


    SUart->RX.BitNum = SUart->Config.DataBitNum + SUart->Config.StopBitNum;
    if (SUart->Config.PairtyState)
    {
        SUart->RX.BitNum += 1;
    }
    SUart->RX.BitCount = SUart->RX.BitNum;      /*配置接收位数和位数计数，不算起始位*/
    SUart->RX.HalfBitCount = 0;
    SUart->RX.Data = 0x0000;
    SUart->Config.RxEnable = UART_DISABLE;
    //以下初始化使用的定时器
	

}



/*******************************************************************************
*  函数名： OD_SimulateUartInit
*  输  入:  无
*  输  出:  无
*  功能说明： 串口的初始化
*******************************************************************************/
void OD_SimulateUartInit(SimulateUart_STR *SUart)
{
    od_SimulateUartPortConfig(SUart);
    od_SimulateUartModeConfig(SUart);

}

/*******************************************************************************
*  函数名： OD_SimulateUartDetectStart
*  输  入:  无
*  输  出:  无
*  功能说明： 串口的接收起始位
*******************************************************************************/
void OD_SimulateUartDetectStart(SimulateUart_STR *SUart)
{
    if (SUart->RX.BitCount == SUart->RX.BitNum)      /*如果接收状态为空闲*/
    {
        SUart->Config.RxEnable = UART_ENABLE;        /*使能接收标志*/
        SUart->RX.Data = 0;
        SUart->RX.ErrorFlag = 0;
    }
}


/*******************************************************************************
*  函数名： OD_SimulateUartMain
*  输  入: 无
*  输  出:  无
*  功能说明： 串口收发的主程序
*******************************************************************************/
void OD_SimulateUartMain(SimulateUart_STR *SUart)
{
    if (SUart->Config.RxEnable)         /*接收使能*/
    {
        SUart->RX.HalfBitCount++;

        if (SUart->RX.BitCount == SUart->RX.BitNum)     /*接到收起始位*/
        {
            if (SUart->RX.HalfBitCount >= 3)       /*接收第一个数据位*/
            {
                if (HAL_GPIO_ReadPin(SUart->Rx_GPIOx, SUart->Rx_GPIO_Pin))      /*读取数据的值为1*/
                {
                    SUart->RX.Data |= (1 << (SUart->RX.BitNum - 1));
                }
                SUart->RX.BitCount--;
                SUart->RX.HalfBitCount = 0;
            }
        }
        else                                  /*接收后面的数据位、校验位、停止位*/
        {
            if (SUart->RX.HalfBitCount >= 2)
            {
                SUart->RX.Data >>= 1;                /*左移一位*/
                if (HAL_GPIO_ReadPin(SUart->Rx_GPIOx, SUart->Rx_GPIO_Pin))      /*读取数据的值为1*/
                {
                    SUart->RX.Data |= (1 << (SUart->RX.BitNum - 1));
                }
                SUart->RX.BitCount--;
                SUart->RX.HalfBitCount = 0;
                if (SUart->RX.BitCount == 0)
                {
                    SUart->RX.BitCount = SUart->RX.BitNum;  /*把数据位配置成设置的长度*/
                    SUart->Config.RxEnable = UART_DISABLE;

                    OD_EnQueue((CirQueue_Str *)&(SUart->SUartQueue), (uint8)SUart->RX.Data);     /*入队*/
                }
            }
        }
    }

    if (SUart->Config.TxEnable)                        /*使能发送*/
    {
        SUart->TX.HalfBitCount++;

        if (SUart->TX.HalfBitCount >= 2)
        {
            SUart->TX.BitCount--;
            SUart->TX.HalfBitCount = 0;
            if (SUart->TX.BitCount == 0)
            {
                SUart->Config.TxEnable = UART_DISABLE;
                return;
            }

            if (SUart->TX.Data & 0x01)         /*发送第一位数据位*/
            {
                HAL_GPIO_WritePin(SUart->Tx_GPIOx, SUart->Tx_GPIO_Pin, GPIO_PIN_SET);
            }
            else
            {
                HAL_GPIO_WritePin(SUart->Tx_GPIOx, SUart->Tx_GPIO_Pin, GPIO_PIN_RESET);
            }
            SUart->TX.Data >>= 1;
        }
    }
}


/*******************************************************************************
*  函数名： OD_SimulateUartWriteData
*  输  入: dat：要发送的数据
*  输  出:  无
*  功能说明： 串口数据的发送
*******************************************************************************/
void OD_SimulateUartWriteByte(SimulateUart_STR *SUart,uint8 dat)
{
    uint8 oneBitNum = 0;

    while (SUart->Config.TxEnable == UART_ENABLE);
    if (SUart->Config.PairtyState)
    {
        oneBitNum = OD_OneBitNum(dat, 8);
        if (((SUart->Config.PairtyState == UART_PAIRTY_EVEN) && (oneBitNum % 2 != 0)) || \
                ((SUart->Config.PairtyState == UART_PAIRTY_ODD) && (oneBitNum % 2 == 0)))
        {
            if (SUart->Config.StopBitNum == 1)
            {
                SUart->TX.Data = dat + (0x03 << SUart->Config.DataBitNum);    /*校验位为1，停止位为1*/
                SUart->TX.Data <<= 1;                                        /*最低位为起始位*/
                SUart->TX.BitCount = SUart->Config.DataBitNum + 4;           /*1位起始位，1位校验位，1位停止位，增加一位时间，用于判断停止位发送完成*/
            }
            else
            {
                SUart->TX.Data = dat + (0x07 << SUart->Config.DataBitNum);    /*校验位为1，2位停止位为1*/
                SUart->TX.Data <<= 1;                                          /*最低位为起始位*/
                SUart->TX.BitCount = SUart->Config.DataBitNum + 5;              /*1位起始位，1位校验位，2位停止位，增加一位时间，用于判断停止位发送完成*/
            }
        }
        else
        {
            if (SUart->Config.StopBitNum == 1)
            {
                SUart->TX.Data = dat + (0x02 << SUart->Config.DataBitNum);    /*校验位为0，停止位为1*/
                SUart->TX.Data <<= 1;                                          /*最低位为起始位*/
                SUart->TX.BitCount = SUart->Config.DataBitNum + 4;             /*1位起始位，1位校验位，1位停止位，增加一位时间，用于判断停止位发送完成*/
            }
            else
            {
                SUart->TX.Data = dat + (0x06 << SUart->Config.DataBitNum);    /*校验位为0，2位停止位为1*/
                SUart->TX.Data <<= 1;                                          /*最低位为起始位*/
                SUart->TX.BitCount = SUart->Config.DataBitNum + 5;             /*1位起始位，1位校验位，2位停止位，增加一位时间，用于判断停止位发送完成*/
            }
        }

    }
    else
    {
        if (SUart->Config.StopBitNum == 1)
        {
            SUart->TX.Data = dat + (0x01 << SUart->Config.DataBitNum);    /*无校验位，停止位为1*/
            SUart->TX.Data <<= 1;                                        /*最低位为起始位*/
            SUart->TX.BitCount = SUart->Config.DataBitNum + 3;            /*1位起始位，1位停止位，增加一位时间，用于判断停止位发送完成*/
        }
        else
        {
            SUart->TX.Data = dat + (0x03 << SUart->Config.DataBitNum);    /*无校验位，2位停止位为1*/
            SUart->TX.Data <<= 1;                                        /*最低位为起始位*/
            SUart->TX.BitCount = SUart->Config.DataBitNum + 4;            /*1位起始位，2位停止位，增加一位时间，用于判断停止位发送完成*/
        }
    }
    SUart->Config.TxEnable = UART_ENABLE;    /*使能发送*/
}

/*******************************************************************************
*  函数名： OD_SimulateUartWriteData
*  输  入: sdata：要发送的数据
           len：要发送数据的长度
*  输  出:  无
*  功能说明： 串口数据的发送
*******************************************************************************/
void OD_SimulateUartWriteData(SimulateUart_STR *SUart,uint8 *wdata, uint16 len)
{

    while (len > 0)
    {
        OD_SimulateUartWriteByte(SUart,*wdata++);
        len--;
    }
}

/*******************************************************************************
*  函数名： OD_SimulateUartWriteData
*  输  入: rdata：接收的数据
*  输  出:  len：接收数据的长度
*  功能说明： 串口数据数据读取
*******************************************************************************/
uint16 OD_SimulateUartRead(SimulateUart_STR *SUart,uint8 *rdata)
{
    uint16 len = 0;

    if (OD_QueueEmpty((CirQueue_Str *)&(SUart->SUartQueue)))
    {
        return 0;                        /*数据长度为0*/
    }


    while (OD_QueueEmpty((CirQueue_Str *)&(SUart->SUartQueue)) != 1)
    {
        *rdata++ = (uint8)OD_DeQueue((CirQueue_Str *)&(SUart->SUartQueue));
        len++;
    }

    return len;
}
