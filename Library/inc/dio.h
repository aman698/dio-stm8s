#ifndef __DIO_H
#define __DIO_H

#include "stm8s.h"

void DIO_Init(void);

void Relay1_Set(uint8_t state);
void Relay2_Set(uint8_t state);
void Relay3_Set(uint8_t state);
void Relay4_Set(uint8_t state);
void Relay5_Set(uint8_t state);
void Relay6_Set(uint8_t state);

uint8_t Input1_Read(void);
uint8_t Input2_Read(void);
uint8_t Input3_Read(void);
uint8_t Input4_Read(void);

#endif