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
#ifndef COMMAND_H
#define COMMAND_H

#include <stdint.h>


typedef enum{
  Cmd_None = 0,
  Cmd_SetMode = 1,
  Cmd_GetMode = 2,
  Cmd_SetPara = 3,
  Cmd_GetPara = 4,
}CommandType;

typedef enum{
  Mode_Stop = 0,
  Mode_ConsCurr = 1,
  Mode_ConsVolt = 2,
  Mode_ConsPower = 3,
  Mode_ConsResis = 4,
}TestMode;

extern volatile TestMode mode;
extern volatile uint16_t paras[3];
#define curr      paras[0]
#define stop_vmin paras[1]
#define report_ms paras[2]

void ExecCmd(uint8_t *buff);

#endif
