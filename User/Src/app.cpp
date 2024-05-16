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
#include <stdbool.h>
#include "ssd1306.hpp"
#include "c_i2c.hpp"
#include "calib.h"
#include "ch32v203_delay.h"
#include "stm32f1xx_ll_i2c.h"
#include "stm32f1xx_ll_gpio.h"
#include <cstdio>
#include "conf.h"
#include "smb.h"
#include "command.h"
#include "calib.h"
#include "keys.h"

extern uint32_t update_count;
extern uint32_t sumI256, sumU256;
extern int64_t sumQ, sumE;

static char tmp[24];
const uint16_t volt_sets[] = {900, 1800, 2400, 2700};
const uint16_t curr_sets[] = {100, 200, 500, 1000, 1500, 2000, 2500};


//显示测试中数据
static void update_show_testing(SSD1306 *oled)
{
  while(update_count%16 != 0);
  int32_t mA = sumI_to_mA(sumI256);
  int32_t mV = sumU_to_mV(sumU256, sumI256);
  int32_t mAh = sumQ_to_mAh(sumQ);
  int32_t mWh = sumE_to_mWh(sumE);

  oled->setVHAddr(Horz_Mode, 0, 127, 0, 0);
  snprintf(tmp, 10, "%5d mA", mA);
  oled->text_5x7(tmp);

  oled->setVHAddr(Horz_Mode, 0, 127, 1, 1);
  snprintf(tmp, 10, "%5d mV", mV);
  oled->text_5x7(tmp);

  oled->setVHAddr(Horz_Mode, 0, 127, 2, 2);
  snprintf(tmp, 10, "%5d mAh", mAh);
  oled->text_5x7(tmp);

  oled->setVHAddr(Horz_Mode, 0, 127, 3, 3);
  snprintf(tmp, 10, "%5d mWh", mWh);
  oled->text_5x7(tmp);
}

//设置测试参数
static void setting(SSD1306 *oled)
{
  oled->setVHAddr(Horz_Mode, 0, 127, 4, 4);
  snprintf(tmp, 24, "Up sett, other cancal");
  oled->text_5x7(tmp);
  uint32_t key = Keys_WaitDownRelease();
  if(key != GPIO_PIN_KEY_UP){
    return;
  }

  int set_i = 0; //0:电流,1:电压
  uint32_t val_i[2];
  const uint32_t val_i_max[2] = {
    sizeof(curr_sets)/sizeof(curr_sets[0]),
    sizeof(volt_sets)/sizeof(volt_sets[0]),
  };
  int32_t mV = sumU_to_mV(sumU256, sumI256);

  //默认放电电流为最小预设值
  val_i[0] = 0;
  //默人截止电压为最接近且小于当前值的预设值
  val_i[1] = 0;
  while(val_i[1]+1<sizeof(volt_sets)/sizeof(volt_sets[0]) &&
	volt_sets[val_i[1]+1]<mV){
    val_i[1]++;
  }

  oled->fill(0x00);
  oled->setVHAddr(Horz_Mode, 0, 127, 4, 4);
  snprintf(tmp, 10, "setting");
  oled->text_5x7(tmp);
  oled->setVHAddr(Horz_Mode, 0, 127, 0, 0);
  snprintf(tmp, 10, "%5d mA", curr_sets[val_i[0]]);
  oled->text_5x7(tmp);
  oled->setVHAddr(Horz_Mode, 0, 127, 1, 1);
  snprintf(tmp, 10, "%5d mV", volt_sets[val_i[1]]);
  oled->text_5x7(tmp);

  while(set_i < 2){
    uint32_t key = Keys_WaitDownRelease();
    if(key){
      bool upd = false;
      if(key & GPIO_PIN_KEY_OK){
	set_i++;
	continue;
      }
      if(key & GPIO_PIN_KEY_UP){
	if(val_i[set_i]+1 < val_i_max[set_i]){
	  val_i[set_i]++;
	  upd = true;
	}
      }
      if(key & GPIO_PIN_KEY_DOWN){
	if(val_i[set_i] > 0){
	  val_i[set_i]--;
	  upd = true;
	}
      }
      if(upd){
	if(set_i == 0){
	  oled->setVHAddr(Horz_Mode, 0, 127, 0, 0);
	  snprintf(tmp, 10, "%5d mA", curr_sets[val_i[set_i]]);
	  oled->text_5x7(tmp);
	}
	if(set_i == 1){
	  oled->setVHAddr(Horz_Mode, 0, 127, 1, 1);
	  snprintf(tmp, 10, "%5d mV", volt_sets[val_i[set_i]]);
	  oled->text_5x7(tmp);
	}
      }
    }
  }
  curr_set = mA_to_sumI(curr_sets[val_i[0]])/16;
  stop_vmin = mV_mA_to_sumU(volt_sets[val_i[1]], curr_sets[val_i[0]])/16;
  mode = Mode_ConsCurr;
}

int app()
{
  C_I2C ci2c;
  ci2c.Instance             = I2Cx_OLED;
  ci2c.Init.ClockSpeed      = I2C_CLOCKSPEED_OLED;
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

  uint32_t ok_cnt = 0;
  while(1){
    Delay_ms(100);
    update_show_testing(&oled);
    if(Keys_IsKeyDown(GPIO_PIN_KEY_OK)){
      ok_cnt++;
    }
    if(ok_cnt > 10){
      //长按OK键进入设置
      ok_cnt = 0;
      Keys_WaitRelease();
      setting(&oled);
      oled.fill(0x00);
    }
  }
}
