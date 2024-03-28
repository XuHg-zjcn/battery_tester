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
#ifndef CALIB_H
#define CALIB_H

#include <stdint.h>

#define mA_LSB (3299.0/4096/256)
#define mV_LSB (5020.0/4096/256)
#define mA_bias 33.0
#define mV_bias -22.0
#define Ohm_Line 0.163

#define fCORE 72000000.0
#define fSAMP (fCORE/256/256)
#define mAh_LSB (mA_LSB/fSAMP/3600)
#define mWh_LSB (mA_LSB*mV_LSB/fSAMP/3600/1000*(1ULL<<16))

#ifdef __cplusplus
extern "C"{
#endif

int32_t sumI_to_mA(uint32_t sumI);
int32_t sumU_to_mV(uint32_t sumU, uint32_t sumI);
int32_t sumQ_to_mAh(int64_t sumQ);
int32_t sumE_to_mWh(int64_t sumE);

#ifdef __cplusplus
}
#endif

#endif
