/*************************************************************************
 *  电池测试仪数据标定和校准
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
#include "calib.h"
#include "ops64.h"
#include <math.h>

const uint32_t I_coef = (uint32_t)(mA_LSB*(1ULL<<32));
const uint32_t U_coef = (uint32_t)(mV_LSB*(1ULL<<32));
const int32_t I_offset = (int32_t)(mA_bias/mA_LSB);
const int32_t U_offset = (int32_t)(mV_bias/mV_LSB);
const uint32_t R_coef = (uint32_t)(Ohm_Line*mA_LSB/mV_LSB*(1ULL<<32));
const uint32_t Q_sht = MAX((int32_t)(-floor(log2(mAh_LSB))-1), 0);
const uint32_t Q_coef = (uint32_t)(mAh_LSB*(1ULL<<Q_sht)*(1ULL<<32));
const uint32_t E_sht = MAX((int32_t)(-floor(log2(mWh_LSB))-1), 0);
const uint32_t E_coef = (uint32_t)(mWh_LSB*(1ULL<<E_sht)*(1ULL<<32));

int32_t sumI_to_noOffset(uint32_t sumI)
{
  return sumI + I_offset;
}

int32_t sumU_to_noOffset(uint32_t sumU, uint32_t sumI)
{
  return sumU + U_offset + I32xU32_HI32(sumI_to_noOffset(sumI), R_coef);
}

int32_t sumI_to_mA(uint32_t sumI)
{
  return I32xU32_HI32(sumI_to_noOffset(sumI), I_coef);
}

int32_t sumU_to_mV(uint32_t sumU, uint32_t sumI)
{
  return I32xU32_HI32(sumU_to_noOffset(sumU, sumI), U_coef);
}

int32_t sumQ_to_mAh(int64_t sumQ)
{
  return I32xU32_HI32(sumQ>>Q_sht, Q_coef);
}

int32_t sumE_to_mWh(int64_t sumE)
{
  return I32xU32_HI32(sumE>>E_sht, E_coef);
}
