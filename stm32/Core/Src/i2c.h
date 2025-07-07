#ifndef __I2C_H
#define __I2C_H

#include "main.h"

void I2CStart(void);
void I2CStop(void);
unsigned char I2CWaitAck(void);
void I2CSendAck(void);
void I2CSendNotAck(void);
void I2CSendByte(unsigned char cSendByte);
unsigned char I2CReceiveByte(void);
void I2CInit(void);
void write_mcp4017(uint8_t data);
uint8_t read_mcp4017(void);
uint8_t read_PCF8574(void);
void for_delay_us(uint32_t us);
void delay1(unsigned int n);
#endif
