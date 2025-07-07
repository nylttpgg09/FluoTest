#include "em1365.h"
#include "string.h"


/*
7E 00 08 01 00 00 F5 CB 23
CPU发出7E 00 08 01 00 02 01 02 DA，启动检测
em1365返回02 00 00 01 00 33 31
随后返回检测到的编号30 35 32 33 30 31 0D 0A，编号为052301

如果没有条码，15S后红灯关闭，不再返回

从串口3发出，9600

*/
uint8_t read_begin[] = {0x7E, 0x00, 0x08, 0x01, 0x00, 0x00, 0xF5, 0xCB, 0x23};
uint8_t read_start[] = {0x7E, 0x00, 0x08, 0x01, 0x00, 0x02, 0x01, 0x02, 0xDA};
uint8_t read_buf[100];
void UartSendByte(UART_HandleTypeDef huart, uint8_t *p_data, uint32_t uiSize);
void  Transmit_Data(UART_HandleTypeDef huart, uint8_t *data, uint32_t len);
void  Transmit_Str(UART_HandleTypeDef huart, uint8_t *data);
extern UART_HandleTypeDef huart3;
#define BUFFER_SIZE 200
extern uint8_t u3flag;
extern uint8_t u3count;
extern uint8_t u3buf[BUFFER_SIZE];

void code_init()
{
    int try = 0;
    int delay = 0;
    Transmit_Data(huart3, read_begin, sizeof(read_begin));
    //HAL_Delay(300);
    memset(u3buf, 0, 100);
    u3count = 0;
    u3flag = 0;
    while (u3flag == 0)
    {
        HAL_Delay(100);
        delay++;
        if (delay == 10)//1秒
        {
            
            break;

        }
    }
    if (u3flag)
    {
        printf("init code rec:");
        for (char i = 0; i < u3count; i++)
            printf("%x ", u3buf[i]);
        printf("\n");
    }
    else
    {
        printf("init code fail,no ack\n");
    }
    memset(u3buf, 0, 100);
    u3count = 0;
    u3flag = 0;
}
uint8_t code_read(uint8_t *code_buf)
{
    uint8_t i, j, delay = 0;
//    UartSendByte(huart3, read_begin, sizeof(read_begin));
//    HAL_Delay(300);
    memset(u3buf, 0, 100);
    u3count = 0;
    u3flag = 0;
    Transmit_Data(huart3, read_start, sizeof(read_start));
    //memset(read_buf, 0, 100);
    //HAL_UART_Receive(&huart3, read_buf, 20, 1000);
    while (u3flag == 0)
    {
        HAL_Delay(100);
        delay++;
        if (delay == 10)//1秒
        {
            break;

        }
    }
	printf("scan read delay=%d\n",delay);
    if (u3count <= 7)
    {
        memset(u3buf, 0, 100);
        u3count = 0;
        u3flag = 0;
        return 0;
    }
    if (delay == 10) //超时
    {
        memset(u3buf, 0, 100);
        u3count = 0;
        u3flag = 0;
        return 0;
    }
    printf("u3rec:");
    for (i = 0; i < u3count; i++) printf("%x ", u3buf[i]);
    printf("\r\n");
    memcpy(read_buf, u3buf, u3count);
    u3count = 0;
    u3flag = 0;
    //locat data
    for (i = 0; i < 100; i++)//查找标志数据 02 00 00 01 00 33 31
    {
        if (read_buf[i] == 0x00 && read_buf[i + 1] == 0x33 && read_buf[i + 2] == 0x31) break;
    }
    if (i == 100) //没有数据
    {
        memset(code_buf, 0, 100);
        u3count = 0;
        u3flag = 0;
        memset(u3buf, 0, 100);
        return 0;
    }
    //i+3为数据起
    i = i + 3;
    j = i;
    for (; i < 100; i++)
    {
        if (read_buf[i] == 0x0d && read_buf[i + 1] == 0x0a) break;
    }
    //i-1为数据止
    read_buf[i] = 0;
    memcpy(code_buf, &read_buf[j], i - j + 1);
//    if (strlen(code_buf) < 5)
//    {
//        return 0;
//    }

    memset(u3buf, 0, 100);
    u3count = 0;
    u3flag = 0;
    return 1;
}

