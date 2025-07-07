/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "i2c.h"
#include "SEGGER_RTT.h"
#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include "41100.h"
#include "em1365.h"
#include "OD_SimulateUart.h"
#include "motor.h"
#include "rx8025.h"


#define LED_ON  HAL_GPIO_WritePin(GPIOA,GPIO_PIN_15,GPIO_PIN_RESET);
#define LED_OFF HAL_GPIO_WritePin(GPIOA,GPIO_PIN_15,GPIO_PIN_SET);

#define EN_ON  HAL_GPIO_WritePin(GPIOA,GPIO_PIN_8,GPIO_PIN_SET);
#define EN_OFF HAL_GPIO_WritePin(GPIOA,GPIO_PIN_8,GPIO_PIN_RESET);

#define CNV_ON  HAL_GPIO_WritePin(GPIOB,GPIO_PIN_12,GPIO_PIN_SET);
#define CNV_OFF HAL_GPIO_WritePin(GPIOB,GPIO_PIN_12,GPIO_PIN_RESET);

//RS485 命令
/*
电机控制 01
启动检测 02
数据返回 03
扫码 04
试剂id查询 05
读ID卡数据 06
*/
#define CMD_MOTOR 01
#define CMD_DETECT 02
#define CMD_READ_DATA 03
#define CMD_SCAN  04
#define CMD_READ_SAMPLE 05
#define CMD_READ_ID 06
#define CMD_SET_TIME 07


FLASH_OBProgramInitTypeDef    OBInit;
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

SimulateUart_STR SUart4;
SimulateUart_STR SUart5;
SimulateUart_STR SUart6;

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
#define BUFFER_SIZE 200
extern uint8_t u1rx_flag;
uint8_t ucRxData;
extern uint8_t u2flag;
extern uint8_t u2count;
extern uint8_t u2buf[BUFFER_SIZE];

extern uint8_t u1count;
extern uint8_t u1rx_buf[BUFFER_SIZE];
extern uint8_t u1tx_buf[BUFFER_SIZE];
extern uint8_t u1rx_flag;
extern uint8_t u3flag;
extern uint8_t u3count;
extern uint8_t u3buf[BUFFER_SIZE];
extern uint32_t time_ms, time_s;

extern uint8_t s_flag;
extern uint16_t total_plus;
uint16_t databuf[1000], *p;
uint8_t codebuf[50];
TIME sys_time;

uint8_t sj_locate; //试剂条位置
uint8_t id_locate; //id卡位置
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
int fputc(int ch, FILE *file)
{
    //HAL_UART_Transmit(&huart1, (uint8_t*)&ch, 1, 1);
    SEGGER_RTT_printf(0, "%c", (uint8_t)ch);
    return ch;
}

void UartSendString(UART_HandleTypeDef huart, uint8_t *p_data)
{
    uint8_t uiSize = strlen((char *)p_data);
    HAL_UART_Transmit(&huart, p_data, uiSize, 100);
}

void UartSendByte(UART_HandleTypeDef huart, uint8_t *p_data, uint32_t uiSize)
{
    HAL_UART_Transmit(&huart, p_data, uiSize, 100);
}
void UartSendByteJjw(UART_HandleTypeDef huart, uint8_t *p_data, uint32_t uiSize)
{
    for (; uiSize; uiSize--)
    {
        while (__HAL_UART_GET_FLAG(&huart, UART_FLAG_TC) == RESET);
        huart.Instance->TDR = *p_data & (uint8_t)0xFFU;
        p_data++;
    }
}
void  Transmit_Data(UART_HandleTypeDef huart, uint8_t *data, uint32_t len)
{
    uint32_t i;
    for (i = 0; i < len; i++)
    {
        while (__HAL_UART_GET_FLAG(&huart, UART_FLAG_TC) == RESET);    //ISR, bit 6 (0x40)
        (&huart)->Instance->TDR = data[i];
    }
}
void  Transmit_Str(UART_HandleTypeDef huart, uint8_t *data)
{
    uint8_t *ch = data;
    while ((*ch) != 0)
    {
        while (__HAL_UART_GET_FLAG(&huart, UART_FLAG_TC) == RESET);    //ISR, bit 6 (0x40)
        (&huart)->Instance->TDR = *ch;
        ch++;
    }
}
void FlashLed()
{
    LED_OFF;
    HAL_Delay(100);
    LED_ON;
    HAL_Delay(100);
    LED_OFF;
}

void detect(uint8_t g)
{
    int i;
    init_41100(g);
    HAL_Delay(10);
    init_41100(g);
    p = databuf;
    //电机前进到试剂条
    motor_sleep(0);//no sleep
    motor_run_back();//回初始位置
    motor_dir(1);
    for (i = 0; i < 4500; i++)//向前
    {
        motor_run(150);
        total_plus++;
    }
    EN_ON;
    HAL_Delay(200);
    HAL_TIM_Base_Start_IT(&htim6);//300us 定时，用于激光头读数
    //慢速 300ms，电机84us，1750个周期
    // 改慢，525ms，电机150us，定时525us
    for (i = 0; i < 1750; i++)//向前
    {
        motor_run(150);
        total_plus++;
    }
    while (1) //wait end
    {
        if (p >= (databuf + 1000))
        {
            HAL_TIM_Base_Stop_IT(&htim6);
            break;
        }
    }
    EN_OFF;
    motor_run_back();//回起点
    motor_sleep(1);//sleep
    printf("data:");
//    for (i = 0; i < 1000; i++)
//    {
//        printf("%d ", databuf[i]);

//        HAL_Delay(5);
//    }
}

uint8_t check_rs485(uint8_t *buf, uint8_t size)
{
    uint8_t sum = 0, i = 0;
    for (i = 0; i < size - 1; i++) sum += buf[i];
    if ((buf[0] == 0x5a) && (buf[1] == 0xa5) && (buf[2] == 0x01) && (buf[size - 1] == sum))
        return 1;//ok
    else
        return 0;
}
uint8_t calc_sum(uint8_t buf[], uint16_t size)
{
    uint16_t i;
    uint8_t sum = 0;
    for (i = 0; i < size; i++) sum += buf[i];
    return sum;
}

void rs485_proc(uint8_t *buf, uint8_t size)
{
    uint8_t ackbuf[10] = {0x5a, 0xa5, 0x01};
    uint8_t sum = 0, temp, res, *p;
    uint16_t len, i;
    switch (buf[3])
    {
    case CMD_MOTOR:
        printf("RS485 motor\n");
        motor_run_arg(buf[5], buf[6], buf[7]);
        ackbuf[3] = CMD_MOTOR | 0x80;
        ackbuf[4] = 0x02;
        ackbuf[5] = total_plus / 256;
        ackbuf[6] = total_plus % 256;
        ackbuf[7] = calc_sum(ackbuf, 7);
        Transmit_Data(huart2, ackbuf, 8);

        break;
    case CMD_DETECT:
        printf("RS485 detect\n");
        detect(buf[5]);
        ackbuf[3] = CMD_DETECT | 0x80;
        ackbuf[4] = 0x01;
        ackbuf[5] = 0x00;
        ackbuf[6] = calc_sum(ackbuf, 6);
        Transmit_Data(huart2, ackbuf, 7);
        break;
    case CMD_READ_DATA:
        printf("RS485 read data\n");
        ackbuf[3] = CMD_READ_DATA | 0x80;
        ackbuf[4] = buf[5];
        ackbuf[5] = buf[6];
        temp = calc_sum(ackbuf, 6);
        len = (buf[5] * 256 + buf[6]) * 2;
        p = (uint8_t *)databuf;
        for (i = 0; i < len; i++)
        {
            temp += *p++;
        }
        Transmit_Data(huart2, ackbuf, 6);
        Transmit_Data(huart2, (uint8_t *)databuf, len);
        Transmit_Data(huart2, &temp, 1);
        break;
    case CMD_READ_ID:
        printf("RS485 read id\n");
        break;
    case CMD_READ_SAMPLE:
        printf("RS485 read sample\n");
        sj_locate = read_PCF8574();
        sj_locate = read_PCF8574();
        id_locate = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_1);
        printf("SJ %x %x\r\n", sj_locate, id_locate);
        ackbuf[3] = CMD_READ_SAMPLE | 0x80;
        ackbuf[4] = 2;
        ackbuf[5] = sj_locate;
        ackbuf[6] = id_locate;
        ackbuf[7] = calc_sum(ackbuf, 7);
        Transmit_Data(huart2, ackbuf, 8);
        break;
    case CMD_SCAN:
        printf("RS485 scan code\n");
        i = 0;
        code_read(codebuf);
        while (code_read(codebuf) == 0)//==0 fail
        {
            i++;
            if (i == 3) break;
            HAL_Delay(1000);
        }
        if (i != 3)
        {
            printf("code=%s\r\n", codebuf);
            ackbuf[3] = CMD_SCAN | 0x80;
            ackbuf[4] = strlen(codebuf);
            temp = calc_sum(ackbuf, 5);
            for (i = 0; i < ackbuf[4]; i++)
            {
                temp += codebuf[i];
            }
            Transmit_Data(huart2, ackbuf, 5);
            Transmit_Data(huart2, (uint8_t *)codebuf, strlen(codebuf));
            Transmit_Data(huart2, &temp, 1);
            memset(codebuf, 0, 100);
        }
        else
        {
            printf("scan code fail\r\n");
            ackbuf[3] = CMD_SCAN | 0x90;
            ackbuf[4] = calc_sum(ackbuf, 4);
            Transmit_Data(huart2, ackbuf, 5);
        }
        break;
    case CMD_SET_TIME:
        printf("RS485 set time \n");
        //AL_Delay(500);
        //printf("init RTC :%s %d\n", (res=rx8025_init())?"fail":"success",res);

        if (1)
        {
            //printf("rx8025 init success\n");
            sys_time.year = buf[5] + 2000;
            sys_time.month = buf[6];
            sys_time.day = buf[7];
            sys_time.hour = buf[8];
            sys_time.minute = buf[9];
            sys_time.second = buf[10];
            sys_time.week = buf[11];

            //printf("set RTC :%s\r\n", set_rx8025_time_t(sys_time)?"fail":"success");
            i = 0;
            while (1)
            {
                res = set_rx8025_time_t(sys_time);
                printf("set RTC %d:%s\r\n", i, res ? "fail" : "success");
                if (res) //fail
                {
                    i++;
                    if (i == 3) break;
                    //HAL_Delay(100);
                }
                else
                {
                    break;
                }
            }


            ackbuf[3] = CMD_SET_TIME | 0x80;
            ackbuf[4] = calc_sum(ackbuf, 4);
            Transmit_Data(huart2, ackbuf, 5);

            //get_rx8025_time(&sys_time);

        }
        else
        {
            //printf("rx8025 inti fail\n");
            ackbuf[3] = CMD_SET_TIME | 0x90;
            ackbuf[4] = calc_sum(ackbuf, 4);
            Transmit_Data(huart2, ackbuf, 5);
        }
        break;

    }
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
    /* USER CODE BEGIN 1 */
    int i, len, adc;
    char buf[100];
    float value;

    /* USER CODE END 1 */

    /* MCU Configuration--------------------------------------------------------*/

    /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
    HAL_Init();

    /* USER CODE BEGIN Init */
    //设置Boot0为Gpio.但无效
//    HAL_FLASH_Unlock();
//    HAL_FLASH_OB_Unlock();
//    HAL_FLASHEx_OBGetConfig(&OBInit);
//    OBInit.OptionType = OPTIONBYTE_USER;
//    OBInit.USERType   = OB_USER_nBOOT0;
//    OBInit.USERConfig = OB_nBOOT0_SET;
//    HAL_FLASHEx_OBProgram(&OBInit);
//    OBInit.OptionType = OPTIONBYTE_USER;
//    OBInit.USERType   = OB_USER_nSWBOOT0;
//    OBInit.USERConfig = OB_BOOT0_FROM_PIN;//OB_BOOT0_FROM_OB
//    HAL_FLASHEx_OBProgram(&OBInit);
//    HAL_FLASH_OB_Lock();
//    HAL_FLASH_Lock();
    /* USER CODE END Init */

    /* Configure the system clock */
    SystemClock_Config();

    /* USER CODE BEGIN SysInit */

    SEGGER_RTT_Init();
    SEGGER_RTT_printf(0, "start Mobile System\n");
    printf("\r\n********************** Mobile ************************\r\n");
//    for (i = 0; i < 30; i++)
//    {
//        printf("#");
//        HAL_Delay(100);
//    }

    /* USER CODE END SysInit */

    /* Initialize all configured peripherals */
    MX_GPIO_Init();
    MX_ADC1_Init();
    MX_SPI2_Init();
    MX_USART1_UART_Init();
    MX_USART2_UART_Init();
    MX_USART3_UART_Init();
    MX_TIM6_Init();
    MX_TIM7_Init();
    /* USER CODE BEGIN 2 */
    //I2CInit();
    HAL_TIM_Base_Start_IT(&htim7);//52us 定时，用于模拟串口
    SimUart_Init();//模拟串口
    /* USER CODE END 2 */

    /* Infinite loop */
    /* USER CODE BEGIN WHILE */
    printf("\r\ninit end\r\n");
    //HAL_Delay(1000);
    i = 0;
    u1count = 0;
    u2count = 0;
    u3count = 0;
    total_plus = 0;


    while (1)
    {
		while(1){//PB3 LED测试
			HAL_Delay(500);
			HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, GPIO_PIN_RESET);
			HAL_Delay(500);
			HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, GPIO_PIN_SET);
		}
        if (1) //RS485协议分析处理
        {
            code_init();
            printf("\r\nScan init end\r\n");
            printf("init RTC :%s\n", rx8025_init() ? "fail" : "success");
            Transmit_Str(huart2, "start\n");
            printf("\r\nRTC init end\r\n");
            motor_sleep(0);//no sleep
            motor_run_back();//回到起点
            motor_sleep(1);//sleep
            while (1)
            {
                if (u2flag)
                {
                    u2flag = 0;
                    len = u2count;
                    u2count = 0;
                    //memset(u2buf,0,100);
                    printf("rs485 cmd:");
                    for (i = 0; i < len; i++) printf("%x ", u2buf[i]);
                    printf("\n");
                    if (check_rs485(u2buf, len)) //check OK
                    {
                        printf("check ok\n");
                        rs485_proc(u2buf, len);
                    }
                }
                //HAL_Delay(100);

                if (s_flag)
                {
                    s_flag = 0;
                    //get_rx8025_time(&sys_time);
                    //printf("%d-%d-%d %d:%d:%d w-%d\r\n", sys_time.year, sys_time.month, sys_time.day, sys_time.hour, sys_time.minute, sys_time.second, sys_time.week);
                    printf("motor locate:%d time:%d\r\n", total_plus, time_ms);
                }
            }
        }
        if (0) // 试剂条检测
        {
            while (1)
            {
                HAL_Delay(1000);
                printf("%x\r\n", read_PCF8574());
            }
        }
        if (0) // RX8025 RTC测试
        {
            if (!rx8025_init())
            {
                printf("rx8025 init success\n");
                sys_time.year = 2024;
                sys_time.month = 8;
                sys_time.day = 19;
                sys_time.hour = 10;
                sys_time.minute = 5;
                sys_time.second = 0;
                sys_time.week = 1;
                set_rx8025_time_t(sys_time);
                get_rx8025_time(&sys_time);
                if (sys_time.month == 165)
                {
                    printf("rx8025 REinit\n");
                    sys_time.year = 2024;
                    sys_time.month = 8;
                    sys_time.day = 19;
                    sys_time.hour = 10;
                    sys_time.minute = 5;
                    sys_time.second = 0;
                    sys_time.week = 1;
                    set_rx8025_time_t(sys_time);
                }
            }
            else
            {
                printf("rx8025 inti fail\n");
            }
            while (1)
            {
                get_rx8025_time(&sys_time);
                printf("%d-%d-%d %d:%d:%d w-%d\r\n", sys_time.year, sys_time.month, sys_time.day, sys_time.hour, sys_time.minute, sys_time.second, sys_time.week);
                HAL_Delay(1000);
            }
        }
        if (1) //使用RTT read，控制SCan code
        {
            while (1)
            {
                buf[0] = 0;
                SEGGER_RTT_Read(0, buf, sizeof(buf));
                if (strlen(buf) == 0)
                {
                    printf(".");
                    HAL_Delay(100);
                    continue;
                }
                else
                {
                    printf("RTT input:%s\r\n", buf);
					if (code_read(buf))
					{
						//for (i=0;i<20;i++) printf("%x ",buf[i]);

						printf("code=%s\r\n", buf);
						UartSendString(huart2, buf);
						memset(buf, 0, 100);
						HAL_Delay(2000);
					}

                }
            }
        }
        if (1) //扫码模块测试
        {
            motor_sleep(0);//no sleep
            motor_run_back();//回到起点
            motor_sleep(1);//sleep
            printf("scan code test\n");
            code_init();
            while (1) //EM1365
            {
                if (code_read(buf))
                {
                    //for (i=0;i<20;i++) printf("%x ",buf[i]);

                    printf("code=%s\r\n", buf);
                    UartSendString(huart2, buf);
                    memset(buf, 0, 100);
                    HAL_Delay(2000);
                }
                HAL_Delay(200);
            }
        }
        if (0)//激光头测试，使用定时器读取
        {
            init_41100(0x10);
            p = databuf;
            //电机前进到试剂条
            motor_sleep(0);//no sleep
            motor_run_back();
            motor_dir(1);
            for (i = 0; i < 4500; i++)    motor_run(100);
            EN_ON;
            HAL_Delay(200);
            HAL_TIM_Base_Start_IT(&htim6);//300us 定时，用于激光头读数
            //慢速 300ms
            for (i = 0; i < 1800; i++)    motor_run(84);
            while (1) //wait end
            {
                if (p >= (databuf + 1000))
                {
                    HAL_TIM_Base_Stop_IT(&htim6);
                    break;
                }
            }
            EN_OFF;
            motor_run_back();
            motor_sleep(1);//sleep
            printf("data:");
            for (i = 0; i < 1000; i++)
            {
                printf("%d ", databuf[i]);

                HAL_Delay(5);
            }
            printf("data read TEST %d\r\n", time_ms);
            //while(1);

            while (1);
        }
        if (0)//激光头测试
        {
            init_41100(0x00);
            p = databuf;
            EN_ON;
            for (i = 0; i < 100; i++) //读100个数据
            {
                CNV_ON;
                for_delay_us(13);
                CNV_OFF;
                for_delay_us(1);
                HAL_SPI_Receive(&hspi2, (uint8_t *)p++, 1, 0xff);
                for_delay_us(300);
            }
            EN_OFF;
            printf("data:");
            for (i = 0; i < 100; i++)
            {
                printf("%d ", databuf[i]);
            }
            printf("data read TEST %d\r\n", time_ms);
            while (1);
        }

        while (0) // EN电源控制测试
        {

            EN_ON;
            HAL_Delay(1000);
            EN_OFF;
            HAL_Delay(1000);
            printf("EN TEST %d\r\n", time_ms);
        }
        if (0) // 移动电机测试
        {
            printf("motor TEST %d\r\n", time_ms);
            //先后退
            motor_sleep(0);//no sleep
            motor_run_back();
            //再前进
            motor_sleep(0);//no sleep
            motor_dir(1);//front
            for (i = 0; i < 000; i++)
            {
                motor_run(1000);
            }
            motor_sleep(1);// sleep
            while (0)
            {
                HAL_Delay(1000);
                printf("EN TEST %d\r\n", time_ms);
            }
        }
        if (1) //AD 转换 AD1 in15
        {
            write_mcp4017(0x0);//数值越小，放大倍数越小。 未设置默认最大
            printf("ADC test\r\n");
            while (1)
            {
                HAL_ADC_Start(&hadc1);
                adc = HAL_ADC_GetValue(&hadc1);
                value = adc * 3.3 / 4096;
                printf("adc=%x V=%f\r\n", adc, value);
                HAL_Delay(1000);
            }
        }
        while (0)
        {
            HAL_Delay(200);
            FlashLed();
            printf("%d\n", i++);
        }

        if (1) //串口2 RX 空闲模式 测试
        {
            UartSendString(huart2, "Uart2 send\n");
            while (1)
            {
                if (u2flag)
                {
                    u2flag = 0;
                    printf("Uart2 rec:");
                    for (i = 0; i < u2count; i++) printf("%x ", u2buf[i]);
                    u2count = 0;
                    putchar('\n');
                    motor_sleep(0);//no sleep
                    motor_dir(1);//front
                    for (i = 0; i < 100; i++)
                    {
                        motor_run(1000);
                    }
                    motor_sleep(1);// sleep
                }
                FlashLed();
                printf("%d\r\n", time_ms);

            }
        }
        if (0) //串口1 RX 空闲模式 测试
        {
            UartSendString(huart1, "Uart1 send\n");
            while (1)
            {
                if (u1rx_flag)
                {
                    u1rx_flag = 0;
                    printf("Uart1 rec:");
                    for (i = 0; i < u1count; i++) printf("%x ", u1rx_buf[i]);
                    u1count = 0;
                    putchar('\n');
                }
                FlashLed();
                printf("%d\r\n", time_ms);

            }
        }
        if (0) //串口3 RX 空闲模式 测试
        {
            UartSendString(huart3, "Uart3 send\n");
            while (1)
            {
                if (u3flag)
                {
                    u3flag = 0;
                    printf("Uart3 rec:");
                    for (i = 0; i < u3count; i++) printf("%x ", u3buf[i]);
                    u3count = 0;
                    putchar('\n');
                }
                FlashLed();
                printf("%d\r\n", time_ms);

            }
        }
        if (0) //串口1 RX DMA 空闲模式 测试 失败，放弃
        {
            HAL_UART_Transmit_DMA(&huart1, "Uart1 dma send\n", 20);
            //UartSendString(huart1, "Uart1 send\n");
            while (1)
            {
                if (u1rx_flag)
                {
                    u1rx_flag = 0;
                    printf("Uart1 rec:");
                    for (i = 0; i < u1count; i++) printf("%x ", u1rx_buf[i]);
                    u1count = 0;
                    putchar('\n');
                }
                FlashLed();
                printf("%d\n", i++);
            }
        }
        if (1) //测试模拟串口
        {
            OD_SimulateUartWriteData(&SUart4, "sim uart4\n", 10);
            OD_SimulateUartWriteData(&SUart5, "sim uart5\n", 10);
            OD_SimulateUartWriteData(&SUart6, "sim uart6\n", 10);
            while (1)
            {
                len = OD_SimulateUartRead(&SUart4, buf);
                if (len)
                {
                    OD_SimulateUartWriteData(&SUart4, buf, len);
                    HAL_Delay(100);
                    printf("sim uart4 rec:");
                    for (i = 0; i < len; i++) printf("%x ", buf[i]);
                }
                len = OD_SimulateUartRead(&SUart5, buf);
                if (len)
                {
                    OD_SimulateUartWriteData(&SUart5, buf, len);
                    HAL_Delay(100);
                    printf("sim uart5 rec:");
                    for (i = 0; i < len; i++) printf("%x ", buf[i]);
                }
                FlashLed();
                printf("%d\r\n", time_ms);
            }
        }
        if (1) //电机测试
        {

        }
        if (1) //EM1365扫码测试
        {
            code_read(buf);
            printf("Scan code:%s\n", buf);
        }
        /* USER CODE END WHILE */

        /* USER CODE BEGIN 3 */
    }
    /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
    RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

    /** Configure the main internal regulator output voltage
    */
    HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1_BOOST);
    /** Initializes the RCC Oscillators according to the specified parameters
    * in the RCC_OscInitTypeDef structure.
    */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV2;
    RCC_OscInitStruct.PLL.PLLN = 85;
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
    RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
    RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        Error_Handler();
    }
    /** Initializes the CPU, AHB and APB buses clocks
    */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                                  | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
    {
        Error_Handler();
    }
    /** Initializes the peripherals clocks
    */
    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART1 | RCC_PERIPHCLK_USART2
                                         | RCC_PERIPHCLK_USART3 | RCC_PERIPHCLK_ADC12;
    PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK2;
    PeriphClkInit.Usart2ClockSelection = RCC_USART2CLKSOURCE_PCLK1;
    PeriphClkInit.Usart3ClockSelection = RCC_USART3CLKSOURCE_PCLK1;
    PeriphClkInit.Adc12ClockSelection = RCC_ADC12CLKSOURCE_SYSCLK;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
    {
        Error_Handler();
    }
}

/* USER CODE BEGIN 4 */


void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{

    if (GPIO_Pin == SUart4.Rx_GPIO_Pin) //触发中断
    {
        if (HAL_GPIO_ReadPin(SUart4.Rx_GPIOx, SUart4.Rx_GPIO_Pin) == GPIO_PIN_RESET)
        {
            OD_SimulateUartDetectStart(&SUart4);
        }
    }
    if (GPIO_Pin == SUart5.Rx_GPIO_Pin) //触发中断
    {
        if (HAL_GPIO_ReadPin(SUart5.Rx_GPIOx, SUart5.Rx_GPIO_Pin) == GPIO_PIN_RESET)
        {
            OD_SimulateUartDetectStart(&SUart5);
        }
    }
    if (GPIO_Pin == SUart6.Rx_GPIO_Pin) //触发中断
    {
        if (HAL_GPIO_ReadPin(SUart6.Rx_GPIOx, SUart6.Rx_GPIO_Pin) == GPIO_PIN_RESET)
        {
            OD_SimulateUartDetectStart(&SUart6);
        }
    }
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    static int tim7_count = 0;
    if (htim->Instance == htim7.Instance)//模拟串口
    {
        OD_SimulateUartMain(&SUart4);
        OD_SimulateUartMain(&SUart5);
        OD_SimulateUartMain(&SUart6);
    }
    if (htim->Instance == htim6.Instance)
    {
        CNV_ON;
        for_delay_us(13);
        CNV_OFF;
        for_delay_us(1);
        HAL_SPI_Receive(&hspi2, (uint8_t *)p++, 1, 0xff);
        if (p >= (databuf + 1000))
        {
            HAL_TIM_Base_Stop_IT(&htim6);
        }
    }
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
    /* USER CODE BEGIN Error_Handler_Debug */
    /* User can add his own implementation to report the HAL error return state */
    __disable_irq();
    while (1)
    {
    }
    /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
    /* USER CODE BEGIN 6 */
    /* User can add his own implementation to report the file name and line number,
       ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
    /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
