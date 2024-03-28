/*************************************************************************
 *  电池测试仪C++程序
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
#include "app.h"
#include "ssd1306.hpp"
#include "c_i2c.hpp"
#include "calib.h"
#include "ch32v203_delay.h"
#include <cstdio>

extern uint32_t update_count;
extern uint32_t sumI256, sumU256;
extern int64_t sumQ, sumE;

int app()
{
  C_I2C ci2c;
  ci2c.Instance             = I2C2;
  ci2c.Init.ClockSpeed      = 400000;
  ci2c.Init.DutyCycle       = I2C_DUTYCYCLE_16_9;
  ci2c.Init.OwnAddress1     = 0xFF;
  ci2c.Init.AddressingMode  = I2C_ADDRESSINGMODE_7BIT;
  ci2c.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  ci2c.Init.OwnAddress2     = 0xFF;
  ci2c.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  ci2c.Init.NoStretchMode   = I2C_NOSTRETCH_DISABLE;
  ci2c.set_Clock(400000);
  C_I2C_Dev dev = C_I2C_Dev(&ci2c, Addr_OLED, I2C_MEMADD_SIZE_8BIT);
  dev.set_TransMode(TransTypeStru({TransBlocking, false, false, false, Wait_error}));
  SSD1306 oled = SSD1306(&dev);
  oled.Init();
  oled.fill(0x00);

  char tmp[16];
  while(1){
    Delay_ms(100);
    while(update_count%16 != 0);
    int32_t mA = sumI_to_mA(sumI256);
    int32_t mV = sumU_to_mV(sumU256, sumI256);
    int32_t mAh = sumQ_to_mAh(sumQ);
    int32_t mWh = sumE_to_mWh(sumE);

    oled.setVHAddr(Horz_Mode, 0, 127, 0, 0);
    snprintf(tmp, 10, "%5d mA", mA);
    oled.text_5x7(tmp);

    oled.setVHAddr(Horz_Mode, 0, 127, 1, 1);
    snprintf(tmp, 10, "%5d mV", mV);
    oled.text_5x7(tmp);

    oled.setVHAddr(Horz_Mode, 0, 127, 2, 2);
    snprintf(tmp, 10, "%5d mAh", mAh);
    oled.text_5x7(tmp);

    oled.setVHAddr(Horz_Mode, 0, 127, 3, 3);
    snprintf(tmp, 10, "%5d mWh", mWh);
    oled.text_5x7(tmp);
  }
}
