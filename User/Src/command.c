/*************************************************************************
 *  电池测试仪命令解析
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
#include "command.h"

#define ARRLEN(x)          (sizeof(x)/sizeof(x[0]))
#define CAST_U8ARR(x)      ((uint8_t *)(x))
#define READU16_UNALIGN(x) (CAST_U8ARR(x)[0] + ((CAST_U8ARR(x)[1])<<8))

volatile TestMode mode;
volatile uint16_t paras[7];
uint32_t wave_logfcurr;
int32_t wave_phase;

void SetMode(TestMode new_mode)
{
  if(new_mode == Mode_CurrWave){
    wave_phase = 0;
    if(wave_logdfdt < 0){
      wave_logfcurr = ((uint32_t)wave_logfmax)<<16;
    }else{
      wave_logfcurr = ((uint32_t)wave_logfmin)<<16;
    }
  }
  mode = new_mode;
}

void ExecCmd(uint8_t *buff)
{
  uint8_t *end = buff+buff[0];
  buff++;
  while(buff < end){
    switch((CommandType)buff[0]){
    case Cmd_SetMode:
      SetMode(buff[1]);
      buff += 2;
      break;
    case Cmd_SetPara:
      if(buff[1] < ARRLEN(paras))
        paras[buff[1]] = READU16_UNALIGN(&buff[2]);
      buff += 4;
      break;
    default:
      return;
    }
  }
}
