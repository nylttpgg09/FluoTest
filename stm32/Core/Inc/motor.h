
#include "main.h"

uint8_t motor_isend(void);
uint8_t motor_isbegin(void);
void motor_dir(char d);
void motor_sleep(char d);
void motor_run(uint16_t step);
void motor_run_flag(void);
void motor_run_front(void);
void motor_run_back(void);
void motor_run_time(uint16_t time);
void for_delay_us(uint32_t us);
void motor_run_time_slow(uint16_t time);
void motor_run_arg(uint8_t period,uint8_t plus,uint8_t dir);

