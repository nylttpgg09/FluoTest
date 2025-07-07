/*******************************************************************************
* Copyright (C), 2014-2015, www.njodin.com
*
* 文件名: OD_CircularQueue.c
* 内容简述: 循环队列操作
*
* 文件历史:
* 版本号    日期      作者      说明
* v0.1    2014.7.15  candy    创建文件
*
*******************************************************************************/
#include "OD_CircularQueue.h"

/*******************************************************************************
*  函数名： OD_QueueInit
*  输  入:  queue：队列结构，bufHead：队列数组头，bufSize：最大长度
*  输  出:  无
*  功能说明： 循环队列的初始化
*******************************************************************************/
void OD_QueueInit(CirQueue_Str *queue, uint8 *bufHead, uint16 bufSize)
{
  queue->BuffHead = bufHead;
  queue->BuffSize = bufSize;
  queue->ReadPtr = 0;
  queue->WritePtr = 0;
  queue->Count = 0;
}

/*******************************************************************************
*  函数名： OD_QueueEmpty
*  输  入:  queue：队列结构
*  输  出:  队列是否为空
*  功能说明： 判断队列是否为空
*******************************************************************************/
uint8 OD_QueueEmpty(CirQueue_Str *queue)
{
  return (queue->Count == 0);
}

/*******************************************************************************
*  函数名： OD_QueueFull
*  输  入:  queue：队列结构
*  输  出:  队列是否为满
*  功能说明： 判断队列是否为满
*******************************************************************************/
uint8 OD_QueueFull(CirQueue_Str *queue)
{
  return (queue->Count == queue->BuffSize);
}

/*******************************************************************************
*  函数名： OD_QueueClear
*  输  入:  queue：队列结构
*  输  出:  无
*  功能说明： 循环队列的清除
*******************************************************************************/
void OD_QueueClear(CirQueue_Str *queue)
{
  queue->ReadPtr = 0;
  queue->WritePtr = 0;
  queue->Count = 0;
}

/*******************************************************************************
*  函数名： OD_EnQueue
*  输  入:  queue：队列结构，dat：入队的数据
*  输  出:  无
*  功能说明： 循环队列的入队
*******************************************************************************/
void OD_EnQueue(CirQueue_Str *queue, uint8 dat)
{
//  if (OD_QueueFull(queue))
//  {
//    DEBUG_WARNING("The data is overflow the circular queue\r\n");
//    return;
//  }
  *(queue->BuffHead + queue->WritePtr) = dat;
  __disable_irq();
  queue->Count++;
  queue->WritePtr = (queue->WritePtr + 1) % queue->BuffSize; //循环队列设计，防止内存浪费
  __enable_irq();
}

/*******************************************************************************
*  函数名： OD_DeQueue
*  输  入:  queue：队列结构
*  输  出:  无
*  功能说明： 循环队列的出队
*******************************************************************************/
uint16 OD_DeQueue(CirQueue_Str *queue)
{
  uint8 temp;
  
//  if (OD_QueueEmpty(queue))
//  {
//    DEBUG_WARNING("The queue is empty\r\n");
//    return 0xFFFF;
//  }
  
  temp = *(queue->BuffHead + queue->ReadPtr);  
  __disable_irq();
  queue->Count--;  
  queue->ReadPtr = (queue->ReadPtr + 1) % queue->BuffSize;  
  __enable_irq();
  return temp;
}
