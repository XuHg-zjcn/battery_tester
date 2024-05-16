/*************************************************************************
 *  PID控制
 *  Copyright (C) 2023-2024  Xu Ruijun
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
#include "pid.h"
#include "ops64.h"

void PID_Init(PID_stat *pid, int32_t err0, int32_t output0)
{
  int32_t output = 0;
#if PID_I_EN
  output += I32xU32_HI32(err0, pid->i_k);
#endif

#if PID_P_EN
  pid->p_stat = CLIP(HI32dU32_I64(output0-output, pid->p_k), pid->p_smin, pid->p_smax);
#endif
}

int32_t PID_update(PID_stat *pid, int32_t err)
{
  int32_t output = 0;
#if PID_I_EN
  output += I32xU32_HI32(err, pid->i_k);
#endif

#if PID_D_EN
  int64_t diff = err - pid->err_last;
#if PID_DLPF_EN
  if(pid->df_keep != 0){
    pid->df_stat = I64xU32_HI64(pid->df_stat, pid->df_keep) + (diff * ((1UL<<32)-pid->keep));
  }else{
    pid->df_stat = diff;
  }
  diff = pid->df_stat>>32;
#endif
  output += I64xU32_MI32(diff, pid->d_k);
  err_last = err;
#endif

#if PID_P_EN
  int32_t curr_p = I64xU32_MI32(pid->p_stat, pid->p_k);
  if((err < 0 && output+curr_p > pid->out_min) || (err > 0 && output+curr_p < pid->out_max)){
    int32_t dp = CLIP(err, pid->p_emin, pid->p_emax);
    pid->p_stat = CLIP(pid->p_stat+dp, pid->p_smin, pid->p_smax);
    output += I64xU32_MI32(pid->p_stat, pid->p_k);
  }else{
    output += curr_p;
  }
#endif
  return CLIP(output, pid->out_min, pid->out_max);
}
