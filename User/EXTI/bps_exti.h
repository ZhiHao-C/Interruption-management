#ifndef __BPS_EXTI_H__
#define __BPS_EXTI_H__

#include "stm32f10x.h" 

#define KEY1_INT_PIN          GPIO_Pin_0
#define KEY1_INT_PORT         GPIOA
#define KEY1_INT_CLK          RCC_APB2Periph_GPIOA

#define KEY2_INT_PIN          GPIO_Pin_13
#define KEY2_INT_PORT         GPIOC
#define KEY2_INT_CLK          RCC_APB2Periph_GPIOC

void EXTI_KEY1_Config(void);
void KEY1_KEY2_EXITConfig(void);
#endif
