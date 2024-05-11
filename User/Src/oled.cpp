/*************************************************************************
 *  电池测试仪OLED驱动
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
#include "oled.h"
#include "ssd1306.hpp"
#include "stm32f1xx_ll_bus.h"
#include "stm32f1xx_ll_gpio.h"
#include "c_i2c.hpp"
#include "conf.h"


void OLED_Init()
{
  LL_APB2_GRP1_EnableClock(APB2PER_ADDR2BIT(GPIO_PORT_OLED_SCL) | APB2PER_ADDR2BIT(GPIO_PORT_OLED_SDA));
  
  LL_GPIO_SetPinMode(GPIO_PORT_OLED_SCL, LL_GPIO_PIN_OLED_SCL, LL_GPIO_MODE_ALTERNATE);
  LL_GPIO_SetPinOutputType(GPIO_PORT_OLED_SCL, LL_GPIO_PIN_OLED_SCL, LL_GPIO_OUTPUT_OPENDRAIN);
  LL_GPIO_SetPinPull(GPIO_PORT_OLED_SCL, LL_GPIO_PIN_OLED_SCL, LL_GPIO_PULL_UP);
  LL_GPIO_SetPinSpeed(GPIO_PORT_OLED_SCL, LL_GPIO_PIN_OLED_SCL, LL_GPIO_SPEED_FREQ_LOW);
  
  LL_GPIO_SetPinMode(GPIO_PORT_OLED_SDA, LL_GPIO_PIN_OLED_SDA, LL_GPIO_MODE_ALTERNATE);
  LL_GPIO_SetPinOutputType(GPIO_PORT_OLED_SDA, LL_GPIO_PIN_OLED_SDA, LL_GPIO_OUTPUT_OPENDRAIN);
  LL_GPIO_SetPinPull(GPIO_PORT_OLED_SDA, LL_GPIO_PIN_OLED_SDA, LL_GPIO_PULL_UP);
  LL_GPIO_SetPinSpeed(GPIO_PORT_OLED_SDA, LL_GPIO_PIN_OLED_SDA, LL_GPIO_SPEED_FREQ_LOW);

  ENABLE_CLOCK_BY_ADDR(I2Cx_OLED);
}
