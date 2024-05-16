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
#include "conf.h"
#include "flash_ch32v2.h"

//该结构体仅在该文件使用,不对外暴露
typedef struct{
  uint32_t I_coef;
  uint32_t U_coef;
  int32_t I_offset;
  int32_t U_offset;
  uint32_t R_coef;
  uint32_t Q_coef;
  uint32_t E_coef;
  uint8_t Q_sht;
  uint8_t E_sht;
}Calib_para;

static Calib_para para;

//电流电压累加值和电量能量转换系数(取决于采样频率)
const uint32_t sumI_to_Q_sht = MAX((int32_t)(-floor(log2(Q_integ))-1), 0);
const uint32_t sumI_to_Q_coef = (uint32_t)(Q_integ*(1ULL<<(32+sumI_to_Q_sht)));
const uint32_t sumE_to_E_sht = MAX((int32_t)(-floor(log2(E_integ))-1), 0);
const uint32_t sumE_to_E_coef = (uint32_t)(E_integ*(1ULL<<(32+sumE_to_E_sht)));

const uint32_t calib_addr = 0x08006000;
const Calib_data default_calib = {
  .head = {'C', 'a', 'b', 'd'},
  .ver = 0,
  .resved = 0,
  .I_coef = (uint32_t)(mA_LSB*(1ULL<<32)),
  .U_coef = (uint32_t)(mV_LSB*(1ULL<<32)),
  .I_offset = (int32_t)(mA_bias/mA_LSB),
  .U_offset = (int32_t)(mV_bias/mV_LSB),
  .R_coef = (uint32_t)(Ohm_Line*mA_LSB/mV_LSB*(1ULL<<32)),
  .calib_time = 0,
};

static inline uint32_t readu32_unalign(void *p)
{
    uint32_t data = *(uint8_t *)(p);
    data |= (*(uint8_t *)(p+1))<<8;
    data |= (*(uint8_t *)(p+2))<<16;
    data |= (*(uint8_t *)(p+3))<<24;
    return data;
}

int32_t sumI_to_noOffset(uint32_t sumI)
{
  return sumI + para.I_offset;
}

int32_t sumU_to_noOffset(uint32_t sumU, uint32_t sumI)
{
  return sumU + para.U_offset + I32xU32_HI32(sumI_to_noOffset(sumI), para.R_coef);
}

int32_t sumI_to_mA(uint32_t sumI)
{
  return I32xU32_HI32(sumI_to_noOffset(sumI), para.I_coef);
}

int32_t sumU_to_mV(uint32_t sumU, uint32_t sumI)
{
  return I32xU32_HI32(sumU_to_noOffset(sumU, sumI), para.U_coef);
}

uint32_t mA_to_sumI(uint32_t mA)
{
  uint32_t noOffset_I = HU32dU32_U64(mA, para.I_coef);
  return noOffset_I - para.I_offset;
}

uint32_t mV_mA_to_sumU(uint32_t mV, uint32_t mA)
{
  uint32_t noOffset_U = HU32dU32_U64(mV, para.U_coef);
  uint32_t noOffset_I = HU32dU32_U64(mA, para.I_coef);
  return noOffset_U - para.U_offset - U32xU32_HU32(noOffset_I, para.R_coef);
}

int32_t sumQ_to_mAh(int64_t sumQ)
{
  return I32xU32_HI32(sumQ>>para.Q_sht, para.Q_coef);
}

int32_t sumE_to_mWh(int64_t sumE)
{
  return I32xU32_HI32(sumE>>para.E_sht, para.E_coef);
}

static void Calib_load()
{
  const Calib_data *p = (const Calib_data *)calib_addr;
  para.I_coef = p->I_coef;
  para.U_coef = p->U_coef;
  para.I_offset = p->I_offset;
  para.U_offset = p->U_offset;
  para.R_coef = p->R_coef;

  uint32_t x0 = __CLZ(para.I_coef);
  para.Q_coef = U32xU32_HU32(para.I_coef<<x0, sumI_to_Q_coef);
  para.Q_sht = sumI_to_Q_sht+x0;
  uint32_t x1 = __CLZ(para.U_coef);
  uint32_t tmp = U32xU32_HU32(para.I_coef<<x0, para.U_coef<<x1);
  para.E_coef = U32xU32_HU32(tmp, sumE_to_E_coef);
  para.E_sht = sumE_to_E_sht+x0+x1;
}

void Calib_update(const Calib_data *p)
{
  if(readu32_unalign((void *)&p->head[0]) != *(uint32_t *)&default_calib.head){
    return;
  }
  const uint32_t addr_f = calib_addr;
  const uint32_t addr_p = (uint32_t)p;
  FLASH_WCH_Unlock_Fast();
  FLASH_WCH_EraseFast(addr_f, 256);
  FLASH_WCH_ProgramFast_Init();
  int i=0;
  for(;i<sizeof(Calib_data);i+=4){
    uint32_t data = readu32_unalign((void *)(addr_p+i));
    FLASH_WCH_ProgramFast_BuffLoad4B(addr_f+i, data);
  }
  for(;i<256;i+=4){
    FLASH_WCH_ProgramFast_BuffLoad4B(addr_f+i, 0);
  }
  FLASH_WCH_ProgramFast_start();
  FLASH_WCH_Lock_All();
  Calib_load();
}

void Calib_Init()
{
  const Calib_data *p = (const Calib_data *)calib_addr;
  if(*(uint32_t *)&p->head != *(uint32_t *)&default_calib.head){
    Calib_update(&default_calib);
  }
  Calib_load();
}
