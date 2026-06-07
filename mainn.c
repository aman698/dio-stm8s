#include <string.h>
#include <stdio.h>

#include "stm8s_conf.h"

void main(void)
{
	uint8_t status;

    /* GPIO */

    GPIO_Init(GPIOC, GPIO_PIN_3, GPIO_MODE_OUT_PP_LOW_FAST);

    GPIO_Init(GPIOD, GPIO_PIN_2, GPIO_MODE_IN_PU_NO_IT);
    GPIO_Init(GPIOD, GPIO_PIN_3, GPIO_MODE_IN_PU_NO_IT);
    GPIO_Init(GPIOD, GPIO_PIN_4, GPIO_MODE_IN_PU_NO_IT);
    GPIO_Init(GPIOD, GPIO_PIN_7, GPIO_MODE_IN_PU_NO_IT);

    
}