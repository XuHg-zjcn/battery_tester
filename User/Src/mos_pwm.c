/*************************************************************************
 *  电池测试仪PWM驱动MOS管
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
#include "mos_pwm.h"
#include "stm32f1xx_ll_bus.h"
#include "stm32f1xx_ll_gpio.h"
#include "stm32f1xx_ll_tim.h"

#define PWM_GPIO_PORT          GPIOA
#define PWM_PIN                LL_GPIO_PIN_15

#define TIMx_PWM               TIM2
#define NAMECONN_PWM_TIMx(x)   x##3
#define LL_TIM_CHANNEL_CHx_PWM LL_TIM_CHANNEL_CH1
#define NAMECONN_PWM_CHx(x)    x##1

void MOS_Init()
{
  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_GPIOA);
  LL_GPIO_SetPinMode(PWM_GPIO_PORT, PWM_PIN, LL_GPIO_MODE_ALTERNATE);
  LL_GPIO_SetPinPull(PWM_GPIO_PORT, PWM_PIN, LL_GPIO_PULL_UP);
  LL_GPIO_SetPinSpeed(PWM_GPIO_PORT, PWM_PIN, LL_GPIO_SPEED_FREQ_LOW);

  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_AFIO);
  LL_GPIO_AF_EnableRemap_TIM2();

  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM2);
  LL_TIM_SetAutoReload(TIMx_PWM, 4096-1);
  NAMECONN_PWM_CHx(LL_TIM_OC_SetCompareCH)(TIMx_PWM, 4096);
  LL_TIM_OC_SetMode(TIMx_PWM, LL_TIM_CHANNEL_CHx_PWM, LL_TIM_OCMODE_PWM1);
  LL_TIM_OC_ConfigOutput(TIMx_PWM, LL_TIM_CHANNEL_CHx_PWM, LL_TIM_OCPOLARITY_HIGH | LL_TIM_OCIDLESTATE_HIGH);
  LL_TIM_CC_EnableChannel(TIMx_PWM, LL_TIM_CHANNEL_CHx_PWM);
  LL_TIM_EnableCounter(TIMx_PWM);
}

void MOS_Set(uint16_t x)
{
  NAMECONN_PWM_CHx(LL_TIM_OC_SetCompareCH)(TIMx_PWM, x);
}
