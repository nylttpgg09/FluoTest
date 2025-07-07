/*******************************************************************************
* Copyright (C), 2014-2015, www.njodin.com
*
* �ļ���: OD_SimulateUart.c
* ���ݼ���: ģ�⴮��������
*
* �ļ���ʷ:
* �汾��    ����      ����      ˵��
* v0.1    2014.9.4   candy     �����ļ�
*
*******************************************************************************/

#include "OD_SimulateUart.h"
#include "main.h"


//volatile CirQueue_Str SUartQueue;
//uint8 SUartQueueBuff[OD_SUART_BUFF_SIZE];
/*******************************************************************************
*  �������� od_SimulateUartPortConfig
*  ��  ��:  ��
*  ��  ��:  ��
*  ����˵���� ���ڶ˿�����
*
*******************************************************************************/

void SimUart_Init(void)
{
    //��ʼ��ģ�⴮��,����1-����ģ�⴮�ڵ����ź��ж�,����2-����OD_SimulateUartInit.
//  ģ�⴮�ڶ���Ϊ�ⲿȫ�ֱ���,����ͳһΪ9600
    //�޸Ķ�ʱ���жϣ��Լ�����3��Exti�ж�

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
    //��ʼ��ģ�⴮������
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

    //��ʼ������
    OD_QueueInit((CirQueue_Str *)&(SUart->SUartQueue), SUart->SUartQueueBuff, OD_SUART_BUFF_SIZE);
}


/*******************************************************************************
*  �������� OD_OneBitNum
*  ��  ��:  dat�����������
*           num�����ݵ�λ��
*  ��  ��:  1�ĸ���
*  ����˵���� ����������1�ĸ���
*
*******************************************************************************/
uint8 OD_OneBitNum(uint16 dat, uint8 num)
{
    register uint8 i;
    uint8 bitNum = 0;
    for (i = 0; i < num; i++)
    {
        bitNum += (dat % 2);        /*��ȡ���λ��ֵ*/
        dat >>= 1;                  /*��������*/
    }

    return bitNum;
}

/*******************************************************************************
*  �������� od_SimulateUartModeConfig
*  ��  ��:  ��
*  ��  ��:  ��
*  ����˵���� �������ó���
*******************************************************************************/
static void od_SimulateUartModeConfig(SimulateUart_STR *SUart)
{
//��ʼ�� SUart�ṹ��

    SUart->Config.BaudRate = OD_SUART_BAUD;
    SUart->Config.DataBitNum = OD_SUART_WORD_LEN;
    SUart->Config.PairtyState = OD_SUART_PARITY;
    SUart->Config.StopBitNum = OD_SUART_STOPBIT;
    SUart->Config.TxEnable = UART_ENABLE;
    SUart->Config.RxEnable = UART_ENABLE;

    SUart->TX.BitCount = 0;                    /*���͵����ݼ�������*/
    SUart->TX.HalfBitCount = 0;                /*һλ���ݴ���ʱ���һ���������*/
    SUart->TX.Data = 0x0000;                   /*������������*/
    SUart->Config.TxEnable = UART_DISABLE;     /*����ʹ�ܹر�*/


    SUart->RX.BitNum = SUart->Config.DataBitNum + SUart->Config.StopBitNum;
    if (SUart->Config.PairtyState)
    {
        SUart->RX.BitNum += 1;
    }
    SUart->RX.BitCount = SUart->RX.BitNum;      /*���ý���λ����λ��������������ʼλ*/
    SUart->RX.HalfBitCount = 0;
    SUart->RX.Data = 0x0000;
    SUart->Config.RxEnable = UART_DISABLE;
    //���³�ʼ��ʹ�õĶ�ʱ��
	

}



/*******************************************************************************
*  �������� OD_SimulateUartInit
*  ��  ��:  ��
*  ��  ��:  ��
*  ����˵���� ���ڵĳ�ʼ��
*******************************************************************************/
void OD_SimulateUartInit(SimulateUart_STR *SUart)
{
    od_SimulateUartPortConfig(SUart);
    od_SimulateUartModeConfig(SUart);

}

/*******************************************************************************
*  �������� OD_SimulateUartDetectStart
*  ��  ��:  ��
*  ��  ��:  ��
*  ����˵���� ���ڵĽ�����ʼλ
*******************************************************************************/
void OD_SimulateUartDetectStart(SimulateUart_STR *SUart)
{
    if (SUart->RX.BitCount == SUart->RX.BitNum)      /*�������״̬Ϊ����*/
    {
        SUart->Config.RxEnable = UART_ENABLE;        /*ʹ�ܽ��ձ�־*/
        SUart->RX.Data = 0;
        SUart->RX.ErrorFlag = 0;
    }
}


/*******************************************************************************
*  �������� OD_SimulateUartMain
*  ��  ��: ��
*  ��  ��:  ��
*  ����˵���� �����շ���������
*******************************************************************************/
void OD_SimulateUartMain(SimulateUart_STR *SUart)
{
    if (SUart->Config.RxEnable)         /*����ʹ��*/
    {
        SUart->RX.HalfBitCount++;

        if (SUart->RX.BitCount == SUart->RX.BitNum)     /*�ӵ�����ʼλ*/
        {
            if (SUart->RX.HalfBitCount >= 3)       /*���յ�һ������λ*/
            {
                if (HAL_GPIO_ReadPin(SUart->Rx_GPIOx, SUart->Rx_GPIO_Pin))      /*��ȡ���ݵ�ֵΪ1*/
                {
                    SUart->RX.Data |= (1 << (SUart->RX.BitNum - 1));
                }
                SUart->RX.BitCount--;
                SUart->RX.HalfBitCount = 0;
            }
        }
        else                                  /*���պ��������λ��У��λ��ֹͣλ*/
        {
            if (SUart->RX.HalfBitCount >= 2)
            {
                SUart->RX.Data >>= 1;                /*����һλ*/
                if (HAL_GPIO_ReadPin(SUart->Rx_GPIOx, SUart->Rx_GPIO_Pin))      /*��ȡ���ݵ�ֵΪ1*/
                {
                    SUart->RX.Data |= (1 << (SUart->RX.BitNum - 1));
                }
                SUart->RX.BitCount--;
                SUart->RX.HalfBitCount = 0;
                if (SUart->RX.BitCount == 0)
                {
                    SUart->RX.BitCount = SUart->RX.BitNum;  /*������λ���ó����õĳ���*/
                    SUart->Config.RxEnable = UART_DISABLE;

                    OD_EnQueue((CirQueue_Str *)&(SUart->SUartQueue), (uint8)SUart->RX.Data);     /*���*/
                }
            }
        }
    }

    if (SUart->Config.TxEnable)                        /*ʹ�ܷ���*/
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

            if (SUart->TX.Data & 0x01)         /*���͵�һλ����λ*/
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
*  �������� OD_SimulateUartWriteData
*  ��  ��: dat��Ҫ���͵�����
*  ��  ��:  ��
*  ����˵���� �������ݵķ���
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
                SUart->TX.Data = dat + (0x03 << SUart->Config.DataBitNum);    /*У��λΪ1��ֹͣλΪ1*/
                SUart->TX.Data <<= 1;                                        /*���λΪ��ʼλ*/
                SUart->TX.BitCount = SUart->Config.DataBitNum + 4;           /*1λ��ʼλ��1λУ��λ��1λֹͣλ������һλʱ�䣬�����ж�ֹͣλ�������*/
            }
            else
            {
                SUart->TX.Data = dat + (0x07 << SUart->Config.DataBitNum);    /*У��λΪ1��2λֹͣλΪ1*/
                SUart->TX.Data <<= 1;                                          /*���λΪ��ʼλ*/
                SUart->TX.BitCount = SUart->Config.DataBitNum + 5;              /*1λ��ʼλ��1λУ��λ��2λֹͣλ������һλʱ�䣬�����ж�ֹͣλ�������*/
            }
        }
        else
        {
            if (SUart->Config.StopBitNum == 1)
            {
                SUart->TX.Data = dat + (0x02 << SUart->Config.DataBitNum);    /*У��λΪ0��ֹͣλΪ1*/
                SUart->TX.Data <<= 1;                                          /*���λΪ��ʼλ*/
                SUart->TX.BitCount = SUart->Config.DataBitNum + 4;             /*1λ��ʼλ��1λУ��λ��1λֹͣλ������һλʱ�䣬�����ж�ֹͣλ�������*/
            }
            else
            {
                SUart->TX.Data = dat + (0x06 << SUart->Config.DataBitNum);    /*У��λΪ0��2λֹͣλΪ1*/
                SUart->TX.Data <<= 1;                                          /*���λΪ��ʼλ*/
                SUart->TX.BitCount = SUart->Config.DataBitNum + 5;             /*1λ��ʼλ��1λУ��λ��2λֹͣλ������һλʱ�䣬�����ж�ֹͣλ�������*/
            }
        }

    }
    else
    {
        if (SUart->Config.StopBitNum == 1)
        {
            SUart->TX.Data = dat + (0x01 << SUart->Config.DataBitNum);    /*��У��λ��ֹͣλΪ1*/
            SUart->TX.Data <<= 1;                                        /*���λΪ��ʼλ*/
            SUart->TX.BitCount = SUart->Config.DataBitNum + 3;            /*1λ��ʼλ��1λֹͣλ������һλʱ�䣬�����ж�ֹͣλ�������*/
        }
        else
        {
            SUart->TX.Data = dat + (0x03 << SUart->Config.DataBitNum);    /*��У��λ��2λֹͣλΪ1*/
            SUart->TX.Data <<= 1;                                        /*���λΪ��ʼλ*/
            SUart->TX.BitCount = SUart->Config.DataBitNum + 4;            /*1λ��ʼλ��2λֹͣλ������һλʱ�䣬�����ж�ֹͣλ�������*/
        }
    }
    SUart->Config.TxEnable = UART_ENABLE;    /*ʹ�ܷ���*/
}

/*******************************************************************************
*  �������� OD_SimulateUartWriteData
*  ��  ��: sdata��Ҫ���͵�����
           len��Ҫ�������ݵĳ���
*  ��  ��:  ��
*  ����˵���� �������ݵķ���
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
*  �������� OD_SimulateUartWriteData
*  ��  ��: rdata�����յ�����
*  ��  ��:  len���������ݵĳ���
*  ����˵���� �����������ݶ�ȡ
*******************************************************************************/
uint16 OD_SimulateUartRead(SimulateUart_STR *SUart,uint8 *rdata)
{
    uint16 len = 0;

    if (OD_QueueEmpty((CirQueue_Str *)&(SUart->SUartQueue)))
    {
        return 0;                        /*���ݳ���Ϊ0*/
    }


    while (OD_QueueEmpty((CirQueue_Str *)&(SUart->SUartQueue)) != 1)
    {
        *rdata++ = (uint8)OD_DeQueue((CirQueue_Str *)&(SUart->SUartQueue));
        len++;
    }

    return len;
}
