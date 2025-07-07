#ifndef __OD_CIRCULARQUEUE_H__
#define __OD_CIRCULARQUEUE_H__
#include <stdint.h>
#include <stdio.h>
	typedef unsigned long 	uint32;
	typedef unsigned short  uint16;
	typedef unsigned char 	uint8;
	
typedef struct CircularQueue_STR
{
  uint8   *BuffHead;    //������ͷ
  uint16  WritePtr;     //д��ָ��index
  uint16  ReadPtr;      //��ȡָ��index
  uint16  Count;        //�������ݳ���
  uint16  BuffSize;     //��󻺳�����С
}CirQueue_Str;

extern void OD_QueueInit(CirQueue_Str *queue, uint8 *bufHead, uint16 bufSize);
extern uint8 OD_QueueEmpty(CirQueue_Str *queue);
extern uint8 OD_QueueFull(CirQueue_Str *queue);
extern void OD_QueueClear(CirQueue_Str *queue);
extern void OD_EnQueue(CirQueue_Str *queue, uint8 dat);
extern uint16 OD_DeQueue(CirQueue_Str *queue);
#endif  /*__OD_CIRCULARQUEUE_H__*/
