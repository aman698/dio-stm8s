#include "dio.h"

void DIO_Init(void)
{
    /* ==========================
       Relay Outputs
       ========================== */

    GPIO_Init(GPIOB, GPIO_PIN_3, GPIO_MODE_OUT_PP_LOW_FAST);
    GPIO_Init(GPIOB, GPIO_PIN_2, GPIO_MODE_OUT_PP_LOW_FAST);
    GPIO_Init(GPIOB, GPIO_PIN_1, GPIO_MODE_OUT_PP_LOW_FAST);
    GPIO_Init(GPIOB, GPIO_PIN_0, GPIO_MODE_OUT_PP_LOW_FAST);

    GPIO_Init(GPIOC, GPIO_PIN_3, GPIO_MODE_OUT_PP_LOW_FAST);
    GPIO_Init(GPIOC, GPIO_PIN_4, GPIO_MODE_OUT_PP_LOW_FAST);

    /* ==========================
       Opto Inputs
       Active LOW
       ========================== */

    GPIO_Init(GPIOD, GPIO_PIN_2, GPIO_MODE_IN_PU_NO_IT);
    GPIO_Init(GPIOD, GPIO_PIN_3, GPIO_MODE_IN_PU_NO_IT);
    GPIO_Init(GPIOD, GPIO_PIN_4, GPIO_MODE_IN_PU_NO_IT);
    GPIO_Init(GPIOD, GPIO_PIN_7, GPIO_MODE_IN_PU_NO_IT);

    /* ==========================
       W5500 Control Pins
       ========================== */

    GPIO_Init(GPIOA, GPIO_PIN_3, GPIO_MODE_OUT_PP_HIGH_FAST); // CS

    /* Reset pin */
    GPIO_Init(GPIOE, GPIO_PIN_5, GPIO_MODE_OUT_PP_HIGH_FAST);

    /* COM Selector
       LOW  = RS232
       HIGH = LAN
    */
    GPIO_Init(GPIOC, GPIO_PIN_1, GPIO_MODE_OUT_PP_LOW_FAST);
}

/* ==========================
   Relay Controls
   ========================== */

void Relay1_Set(uint8_t state)
{
    if(state)
        GPIO_WriteHigh(GPIOB, GPIO_PIN_3);
    else
        GPIO_WriteLow(GPIOB, GPIO_PIN_3);
}

void Relay2_Set(uint8_t state)
{
    if(state)
        GPIO_WriteHigh(GPIOB, GPIO_PIN_2);
    else
        GPIO_WriteLow(GPIOB, GPIO_PIN_2);
}

void Relay3_Set(uint8_t state)
{
    if(state)
        GPIO_WriteHigh(GPIOB, GPIO_PIN_1);
    else
        GPIO_WriteLow(GPIOB, GPIO_PIN_1);
}

void Relay4_Set(uint8_t state)
{
    if(state)
        GPIO_WriteHigh(GPIOB, GPIO_PIN_0);
    else
        GPIO_WriteLow(GPIOB, GPIO_PIN_0);
}

void Relay5_Set(uint8_t state)
{
    if(state)
        GPIO_WriteHigh(GPIOC, GPIO_PIN_3);
    else
        GPIO_WriteLow(GPIOC, GPIO_PIN_3);
}

void Relay6_Set(uint8_t state)
{
    if(state)
        GPIO_WriteHigh(GPIOC, GPIO_PIN_4);
    else
        GPIO_WriteLow(GPIOC, GPIO_PIN_4);
}

/* ==========================
   Inputs
   Active LOW
   Return:
   1 = Asserted
   0 = Idle
   ========================== */

uint8_t Input1_Read(void)
{
    return (GPIO_ReadInputPin(GPIOD, GPIO_PIN_2) == RESET);
}

uint8_t Input2_Read(void)
{
    return (GPIO_ReadInputPin(GPIOD, GPIO_PIN_3) == RESET);
}

uint8_t Input3_Read(void)
{
    return (GPIO_ReadInputPin(GPIOD, GPIO_PIN_4) == RESET);
}

uint8_t Input4_Read(void)
{
    return (GPIO_ReadInputPin(GPIOD, GPIO_PIN_7) == RESET);
}