/* ================================================================
   STM8S basic application (W5500 + SPI removed)
   ================================================================

   Note: All W5500 ethernet and SPI code has been removed from this file.

*/

#define STM8S003

#include <string.h>
#include <stdio.h>

#include "stm8s.h"
#include "stm8s_uart1.h"
#include "stm8s_conf.h"

/* ================================================================
   DELAY
   ================================================================ */

static void delay_ms(uint16_t ms)
{
    uint16_t i;
    uint16_t j;

    for(i = 0; i < ms; i++)
    {
        for(j = 0; j < 16000 / 4; j++)
        {
            nop();
        }
    }
}

/* ================================================================
   UART
   ================================================================ */

static void UART_Config(void)
{
    UART1_DeInit();

    UART1_Init(
        9600,
        UART1_WORDLENGTH_8D,
        UART1_STOPBITS_1,
        UART1_PARITY_NO,
        UART1_SYNCMODE_CLOCK_DISABLE,
        UART1_MODE_TXRX_ENABLE
    );

    UART1_Cmd(ENABLE);
}

static void UART_SendString(char *str)
{
    while(*str)
    {
        UART1_SendData8(*str++);

        while(UART1_GetFlagStatus(UART1_FLAG_TXE) == RESET);
    }
}

/* ================================================================
   MAIN
   ================================================================ */

void main(void)
{
    uint8_t in1, in2, in3, in4;
    char msg[32];

    /* GPIO: relay output */
    GPIO_Init(GPIOC, GPIO_PIN_3, GPIO_MODE_OUT_PP_LOW_FAST);

    /* GPIO: inputs */
    GPIO_Init(GPIOD, GPIO_PIN_2, GPIO_MODE_IN_PU_NO_IT);
    GPIO_Init(GPIOD, GPIO_PIN_3, GPIO_MODE_IN_PU_NO_IT);
    GPIO_Init(GPIOD, GPIO_PIN_4, GPIO_MODE_IN_PU_NO_IT);
    GPIO_Init(GPIOD, GPIO_PIN_7, GPIO_MODE_IN_PU_NO_IT);

    /* UART */
    UART_Config();

    UART_SendString("SYSTEM START (W5500 + SPI removed)\r\n");

    while(1)
    {
        in1 = GPIO_ReadInputPin(GPIOD, GPIO_PIN_2);
        in2 = GPIO_ReadInputPin(GPIOD, GPIO_PIN_3);
        in3 = GPIO_ReadInputPin(GPIOD, GPIO_PIN_4);
        in4 = GPIO_ReadInputPin(GPIOD, GPIO_PIN_7);

        sprintf(msg, "IN:%d%d%d%d\r\n", in1, in2, in3, in4);
        UART_SendString(msg);

        delay_ms(500);
    }
}

