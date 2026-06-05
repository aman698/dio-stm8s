/**
  ******************************************************************************
  * @file     stm8s_it.h
  * @author   MCD Application Team
  * @version  V2.1.0
  * @date     18-November-2011
  * @brief    This file contains the headers of the interrupt handlers
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
  ******************************************************************************
  */ 

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __STM8S_IT_H
#define __STM8S_IT_H

/* Includes ------------------------------------------------------------------*/
#include "stm8s.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
#ifdef _COSMIC_
 void _stext(void); /* RESET startup routine */
 INTERRUPT void NonHandledInterrupt(void);
#endif /* _COSMIC_ */

#ifndef _RAISONANCE_
 INTERRUPT void TRAP_IRQHandler(void); /* TRAP */
 INTERRUPT void TLI_IRQHandler(void); /* TLI */
 INTERRUPT void AWU_IRQHandler(void); /* AWU */
 INTERRUPT void CLK_IRQHandler(void); /* CLOCK */
 INTERRUPT void EXTI_PORTA_IRQHandler(void); /* EXTI PORTA */
 INTERRUPT void EXTI_PORTB_IRQHandler(void); /* EXTI PORTB */
 INTERRUPT void EXTI_PORTC_IRQHandler(void); /* EXTI PORTC */
 INTERRUPT void EXTI_PORTD_IRQHandler(void); /* EXTI PORTD */
 INTERRUPT void EXTI_PORTE_IRQHandler(void); /* EXTI PORTE */
 INTERRUPT void SPI_IRQHandler(void); /* SPI */
 INTERRUPT void TIM1_CAP_COM_IRQHandler(void); /* TIM1 CAP/COM */
 INTERRUPT void TIM1_UPD_OVF_TRG_BRK_IRQHandler(void); /* TIM1 UPD/OVF/TRG/BRK */
 INTERRUPT void UART1_TX_IRQHandler(void); /* UART1 TX */
 INTERRUPT void UART1_RX_IRQHandler(void); /* UART1 RX */
 INTERRUPT void I2C_IRQHandler(void); /* I2C */
 INTERRUPT void EEPROM_EEC_IRQHandler(void); /* EEPROM ECC CORRECTION */
#endif /* _RAISONANCE_ */

#endif /* __STM8S_IT_H */

/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
