/*************************************************************************
 *  PID控制
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
#ifndef PID_H
#define PID_H

#include <stdint.h>

#define PID_P_EN 1
#define PID_I_EN 1
#define PID_D_EN 0
#define PID_DLPF_EN 1

typedef struct{
#if PID_P_EN
  int64_t p_stat;
  int64_t p_smin;
  int64_t p_smax;
  int32_t p_emin;
  int32_t p_emax;
  uint32_t p_k;
#endif

#if PID_I_EN
  uint32_t i_k;
#endif

#if PID_D_EN
  int32_t err_last;
#if PID_DLPF_EN
  int64_t df_stat;
  uint32_t df_keep;
#endif
  uint32_t d_k;
#endif

  int32_t out_min;
  int32_t out_max;
}PID_stat;

void PID_Init(PID_stat *pid, int32_t err0, int32_t output0);
int32_t PID_update(PID_stat *pid, int32_t err);

#endif
