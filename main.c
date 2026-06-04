#include "stm8s.h"
#include "stm8s_gpio.h"

void delay(void)
{
    unsigned long i;
    for(i = 30000; i > 0; i--);
}

void main(void)
{
    GPIO_Init(GPIOB,
              GPIO_PIN_4,
              GPIO_MODE_OUT_PP_LOW_FAST);

    while(1)
    {
        GPIO_WriteReverse(GPIOB, GPIO_PIN_4);
        delay();
    }
}

// This is aman lakra