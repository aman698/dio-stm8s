#include "stm8_uart.h"

void UART_Config(void){
    UART1_DeInit();
    UART1_Init(9600, UART1_WORDLENGTH_8D, UART1_STOPBITS_1, UART1_PARITY_NO UART1_SYNCMODE_CLOCK_DISABLE, UART1_MODE_TXRX_ENABLE);
    UART1_Cmd(ENABLE);    
}

void UART_SendString(char *str)
{
    while(*str){
        UART1_SendData(*str++);
        while(UART1_GetFlagStatus(UART_FLAG_TXE) == RESET);
    }
}

void UART_SendHex(uint8_t value)
{
    char buf[5];

    buf[0] = '0';
    buf[1] = 'x';
    buf[2] = "0123456789ABCDEF"[value >> 4];
    buf[3] = "0123456789ABCDEF"[value & 0x0F];
    buf[4] = 0;

    UART_SendString(buf);
}
