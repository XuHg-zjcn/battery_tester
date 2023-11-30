/*******************************************************************************
 * Interrupt handlers for CH32V103
 * Copyright (C) 2023 Xu Ruijun
 *
 * License under BSD 3-clause, see LICENSE.BSD3 file
 ******************************************************************************/
#include "ch32v103_it.h"
#include "stm32f1xx_ll_exti.h"
#include "stm32f1xx_ll_gpio.h"
#include "stm32f1xx_hal_gpio.h"

//void Default_Handler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
//void EXTI0_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
//void Default_Handler(void) __attribute__((naked()));
void EXTI0_IRQHandler(void) __attribute__((naked()));
//void Default_Handler(void) __attribute__((interrupt()));
//void EXTI0_IRQHandler(void) __attribute__((interrupt()));

/*void Default_Handler(void)
{
  while(1){
  }
}*/

void EXTI0_IRQHandler(void)
{
  if(LL_EXTI_IsActiveFlag_0_31(LL_EXTI_LINE_0) != RESET)
  {
    LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_0);

    LL_GPIO_TogglePin(GPIOC, LL_GPIO_PIN_13);
    HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
  }
}
