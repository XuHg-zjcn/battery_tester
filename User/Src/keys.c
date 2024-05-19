/*************************************************************************
 *  电池测试仪按键相关
 *  Copyright (C) 2024  Xu Ruijun
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
#include "keys.h"
#include "conf.h"
#include "stm32f1xx_ll_bus.h"
#include "stm32f1xx_ll_gpio.h"
#include "stm32f1xx_hal_gpio.h"

#if KEYS_EN
void Keys_Init()
{
  ENABLE_CLOCK_BY_ADDR(GPIO_PORT_KEYS);

  LL_GPIO_SetPinMode(GPIO_PORT_KEYS, LL_GPIO_PIN_KEY_OK, LL_GPIO_MODE_INPUT);
  LL_GPIO_SetPinPull(GPIO_PORT_KEYS, LL_GPIO_PIN_KEY_OK, LL_GPIO_PULL_UP);

  LL_GPIO_SetPinMode(GPIO_PORT_KEYS, LL_GPIO_PIN_KEY_UP, LL_GPIO_MODE_INPUT);
  LL_GPIO_SetPinPull(GPIO_PORT_KEYS, LL_GPIO_PIN_KEY_UP, LL_GPIO_PULL_UP);

  LL_GPIO_SetPinMode(GPIO_PORT_KEYS, LL_GPIO_PIN_KEY_DOWN, LL_GPIO_MODE_INPUT);
  LL_GPIO_SetPinPull(GPIO_PORT_KEYS, LL_GPIO_PIN_KEY_DOWN, LL_GPIO_PULL_UP);
}

int Keys_IsKeyDown(uint32_t key)
{
    if((~LL_GPIO_ReadInputPort(GPIO_PORT_KEYS))&key){
      return 1;
    }else{
      return 0;
    }
}

uint32_t Keys_WaitKeydown()
{
  uint32_t key;
  do{
    key = (~LL_GPIO_ReadInputPort(GPIO_PORT_KEYS))&GPIO_PIN_KEY_ALL;
  }while(!key);
  return key;
}

uint32_t Keys_WaitRelease()
{
  uint64_t t0 = SysTick_GetCNT_Safe();
  while((~LL_GPIO_ReadInputPort(GPIO_PORT_KEYS))&GPIO_PIN_KEY_ALL);
  uint64_t t1 = SysTick_GetCNT_Safe();
  return t1-t0;
}

uint32_t Keys_WaitDownRelease()
{
  uint32_t key, time;
  do{
    key = Keys_WaitKeydown();
    time = Keys_WaitRelease();
  }while(time < 90000); //10ms
  return key;
}
#endif
