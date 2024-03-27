/*************************************************************************
 *  电池测试仪固件主程序文件
 *  Copyright (C) 2023  Xu Ruijun
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *************************************************************************/
#include "stm32f1xx_ll_utils.h"
#include "stm32f1xx_ll_system.h"
#include "stm32f1xx_ll_rcc.h"
#include "stm32f1xx_ll_bus.h"
#include "stm32f1xx_ll_gpio.h"
#include "stm32f1xx_ll_usart.h"
#include "mos_pwm.h"
#include "adc.h"
#include "usart.h"
#include "pid.h"
#include "usart.h"
#include "command.h"
#include "oled.h"

#define LED1_GPIO_PORT GPIOC
#define LED1_PIN       LL_GPIO_PIN_13

uint16_t data[3];

PID_stat pid = {
  .p_stat = 1000*4096,
  .p_smin = 1000*1000,
  .p_smax = 1000*4096,
  .p_emin = -1000,
  .p_emax = 1000,
  .p_k = 0.001*(1ULL<<32),

  .i_k = 0.007*(1ULL<<32),

  .out_min = 100,
  .out_max = 4096,
};

void SystemClock_Config(void);

void LED_Init()
{
  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_GPIOC);
  LL_GPIO_SetPinMode(LED1_GPIO_PORT, LED1_PIN, LL_GPIO_MODE_OUTPUT);
  LL_GPIO_ResetOutputPin(LED1_GPIO_PORT, LED1_PIN);
}

void main()
{
  SystemClock_Config();
  MOS_Init();
  LED_Init();
  USART_Init();
  //PID_Init(&pid, -400, 4000);
  OLED_Init();
  ADC_Init();
  data[0] = 65535;
  while(1){
  }
}

/**
 * Source from
 *     repo: https://github.com/STMicroelectronics/STM32CubeF1/
 *     file: Projects/STM32F103RB-Nucleo/Examples_LL/TIM/TIM_PWMOutput/Src/main.c
  * @brief  System Clock Configuration
  *         The system Clock is configured as follow :
  *            System Clock source            = PLL (HSE)
  *            SYSCLK(Hz)                     = 72000000
  *            HCLK(Hz)                       = 72000000
  *            AHB Prescaler                  = 1
  *            APB1 Prescaler                 = 2
  *            APB2 Prescaler                 = 1
  *            HSE Frequency(Hz)              = 8000000
  *            PLLMUL                         = 9
  *            Flash Latency(WS)              = 2
  * @param  None
  * @retval None
  */
void SystemClock_Config(void)
{
  /* Set FLASH latency */
  LL_FLASH_SetLatency(LL_FLASH_LATENCY_2);

  /* Enable HSE oscillator */
  LL_RCC_HSE_Enable();
  while(LL_RCC_HSE_IsReady() != 1);

  /* Main PLL configuration and activation */
  LL_RCC_PLL_ConfigDomain_SYS(LL_RCC_PLLSOURCE_HSE_DIV_1, LL_RCC_PLL_MUL_9);

  LL_RCC_PLL_Enable();
  while(LL_RCC_PLL_IsReady() != 1);

  /* Sysclk activation on the main PLL */
  LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
  LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL);
  while(LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL);

  /* Set APB1 & APB2 prescaler*/
  LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_2);
  LL_RCC_SetAPB2Prescaler(LL_RCC_APB2_DIV_1);

  /* Update CMSIS variable (which can be updated also through SystemCoreClockUpdate function) */
  LL_SetSystemCoreClock(72000000);
}
