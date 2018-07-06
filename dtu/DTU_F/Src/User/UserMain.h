#ifndef __USERMAIN_H__
#define __UAERMAIN_H__
#include "stm32f1xx_hal.h"
#include "array.h"


#define UART_CHAN_DEBUG 2
#define UART_CHAN_EC20  3
#define UART_CHAN_HOST  1

extern T_Array ArrayUartRX[3];
extern T_Array ArrayUartTX[1];

void DebugOutStr(uint8_t *buf,uint16_t tLen);

void UartSendStr(uint8_t chan,uint8_t *tBuf,uint16_t tLen);
void DebugOutHexStr(uint8_t *buf,uint16_t tLen);
void DebugOutHexStr_fast(uint8_t *buf,uint16_t tLen);

#endif


