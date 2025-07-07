#include "motor.h"
#include "stdio.h"
/*
上电自检，向前0.13秒，退回。

界面显示时自检，向前到终点，退回。

*/

uint16_t total_plus = 0;

uint8_t motor_isbegin(void) //起点 ，到位5V， 平时0V
{
    return HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13) == GPIO_PIN_SET; //Op1 PC13
}

uint8_t motor_isend(void)
{
    return 1;
}

void motor_dir(char d)
{
    if (d) //向前
    {
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_RESET); //PA0为高时，向前。8825的dir为5v时，正向
    }
    else   //向后
    {
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_SET);
    }
}

void motor_sleep(char d)
{
    if (d) //休眠
    {
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_15, GPIO_PIN_RESET); //PC14为低时，。8825的sleep为0v时，休眠
    }
    else   //运行
    {
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_15, GPIO_PIN_SET);
    }
}

void for_delay_us(uint32_t us)
{
    uint32_t Delay = us * 33;
    do
    {
        __NOP();
    }
    while (Delay --);
}

void motor_run(uint16_t step)
{
    //step产生脉冲
    if (1)
    {
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_14, GPIO_PIN_RESET);
        //HAL_Delay(1);
        for_delay_us(step);
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_14, GPIO_PIN_SET);
        //HAL_Delay(1);
        for_delay_us(step);

    }
}

void motor_run_flag(void)
{
    //step产生脉冲
    uint16_t i;
    for (i = 0; i < 100; i++) motor_run(250);
    while (1)
    {
        if (motor_isbegin() || motor_isend())
        {
            HAL_GPIO_WritePin(GPIOC, GPIO_PIN_14, GPIO_PIN_RESET);
            break;
        }
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_14, GPIO_PIN_RESET);
        //HAL_Delay(1);
        for_delay_us(200);
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_14, GPIO_PIN_SET);
        //HAL_Delay(1);
        for_delay_us(200);

    }
}

void motor_run_front(void)
{
    motor_dir(1);
    while (1)
    {
        if (motor_isend())
        {
            HAL_GPIO_WritePin(GPIOC, GPIO_PIN_14, GPIO_PIN_RESET);
            break;
        }
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_14, GPIO_PIN_RESET);
        //HAL_Delay(1);
        for_delay_us(200);
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_14, GPIO_PIN_SET);
        //HAL_Delay(1);
        for_delay_us(200);

    }
}
void motor_run_back(void)
{
    int i = 0;
    motor_dir(0);
    while (1)
    {
        if (motor_isbegin())
        {
            HAL_GPIO_WritePin(GPIOC, GPIO_PIN_14, GPIO_PIN_RESET);
            printf("back motor :%d\r\n", i);
            total_plus = 0;
            break;
        }
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_14, GPIO_PIN_RESET);
        //HAL_Delay(1);
        for_delay_us(80);
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_14, GPIO_PIN_SET);
        //HAL_Delay(1);
        for_delay_us(80);
        i++;
        total_plus--;

    }
}
extern uint32_t time_ms;
void motor_run_time(uint16_t time)
{
    uint16_t i;
    uint32_t time_end = time + time_ms;
    for (i = 0; i < 100; i++) motor_run(250);
    while (1)
    {

        if (time_end == time_ms || motor_isbegin() || motor_isend())
        {
            HAL_GPIO_WritePin(GPIOC, GPIO_PIN_14, GPIO_PIN_RESET);
            break;
        }
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_14, GPIO_PIN_RESET);
        //HAL_Delay(1);
        for_delay_us(250);
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_14, GPIO_PIN_SET);
        //HAL_Delay(1);
        for_delay_us(250);

    }
}
void motor_run_time_slow(uint16_t time)
{
    uint16_t i;
    uint32_t time_end = time + time_ms;
    for (i = 0; i < 100; i++) motor_run(250);
    while (1)
    {

        if (time_end == time_ms || motor_isbegin() || motor_isend())
        {
            HAL_GPIO_WritePin(GPIOC, GPIO_PIN_14, GPIO_PIN_RESET);
            break;
        }
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_14, GPIO_PIN_RESET);
        //HAL_Delay(1);
        for_delay_us(350);
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_14, GPIO_PIN_SET);
        //HAL_Delay(1);
        for_delay_us(350);

    }
}

void motor_run_arg(uint8_t period, uint8_t plus, uint8_t dir)
{
    int i;
    motor_sleep(0);
    motor_dir(dir);
    if (dir) //front
    {
        for (i = 0; i < plus * 100; i++)
        {
            motor_run(period);
            total_plus++;
        }
    }
    else
    {
        for (i = 0; i < plus * 100; i++)
        {
            if (motor_isbegin())
            {
                total_plus = 0;
                break;
            }
            motor_run(period);
            total_plus--;
        }
    }
    motor_sleep(1);
}

