/*******************************************************************************
 * Interrupt handlers for CH32V203
 * Copyright (C) 2023 Xu Ruijun
 *
 * License under BSD 3-clause, see LICENSE.BSD3 file
 ******************************************************************************/
#include "ch32v203_it.h"
#include "stm32f1xx_ll_exti.h"
#include "stm32f1xx_ll_gpio.h"
#include "stm32f1xx_hal_gpio.h"

void EXTI0_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
