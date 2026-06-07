#ifndef __STM8_UART
#define __STM8_UART

#include "stm8s_uart1.h"

void UART_Config(void);
void UART_SendString(char *str);
void UART_SendHex(uint8_t value);

#endif