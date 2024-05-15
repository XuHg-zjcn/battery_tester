/*************************************************************************
 * Flash driver for CH32V2x, CH32V3x, CH32F2x
 * Copyright (C) 2024  Xu Ruijun
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *************************************************************************/
#if defined(CH32V203x8) && !defined(FLASH_CH32V2_H)
#define FLASH_CH32V2_H
#include "stm32f1xx.h"

//寄存器名称来源于https://www.wch.cn/downloads/CH32FV2x_V3xRM_PDF.html, 版本2.1
#define KEY1 0x45670123
#define KEY2 0xCDEF89AB

#define FLASH_STATR_BSY      (1ULL<<0)
#define FLASH_STATR_WR_BUSY  (1ULL<<1)
#define FLASH_STATR_WRPRTERR (1ULL<<4)
#define FLASH_STATR_EOP      (1ULL<<5)
#define FLASH_STATR_EHMODS   (1ULL<<7)

#define FLASH_CTLR_PG        (1ULL<<0)
#define FLASH_CTLR_SER       (1ULL<<1)
#define FLASH_CTLR_MER       (1ULL<<2)
#define FLASH_CTLR_OBPG      (1ULL<<4)
#define FLASH_CTLR_OBER      (1ULL<<5)
#define FLASH_CTLR_STRT      (1ULL<<6)
#define FLASH_CTLR_LOCK      (1ULL<<7)
#define FLASH_CTLR_OBWRE     (1ULL<<9)
#define FLASH_CTLR_ERRIE     (1ULL<<10)
#define FLASH_CTLR_EOPIE     (1ULL<<12)
#define FLASH_CTLR_FLOCK     (1ULL<<15)
#define FLASH_CTLR_FTPG      (1ULL<<16)
#define FLASH_CTLR_FTER      (1ULL<<17)
#define FLASH_CTLR_BER32     (1ULL<<18)
#define FLASH_CTLR_BER64     (1ULL<<19)
#define FLASH_CTLR_PGSTRT    (1ULL<<21)
#define FLASH_CTLR_RSENACT   (1ULL<<22)
#define FLASH_CTLR_EHMOD     (1ULL<<24)
#define FLASH_CTLR_SCKMOD    (1ULL<<25)

#define FLASH_OBR_OBERR      (1ULL<<0)
#define FLASH_OBR_RDPRT      (1ULL<<1)
#define FLASH_OBR_IWDGSW     (1ULL<<2)
#define FLASH_OBR_STOPRST    (1ULL<<3)
#define FLASH_OBR_STANDYRST  (1ULL<<4)

typedef struct{
  __IO uint32_t resv1;
  __IO uint32_t KEYR;
  __IO uint32_t OBKEYR;
  __IO uint32_t STATR;
  __IO uint32_t CTLR;
  __IO uint32_t ADDR;
  __IO uint32_t resv2;
  __IO uint32_t OBR;
  __IO uint32_t WPR;
  __IO uint32_t MODEKEYR;
}FLASH_WCH_TypeDef;

#define FLASH_WCH               ((FLASH_WCH_TypeDef *)FLASH_R_BASE)


#ifdef __cplusplus
extern "C"{
#endif

static inline int FLASH_WCH_IsBusy()
{
  return (FLASH_WCH->STATR & FLASH_STATR_BSY)?1:0;
}

void FLASH_WCH_Unlock_Fast();
void FLASH_WCH_Lock_All();
void FLASH_WCH_ProgramFast_Init();
void FLASH_WCH_ProgramFast_BuffLoad4B(uint32_t addr, uint32_t data);
void FLASH_WCH_ProgramFast_start();
void FLASH_WCH_EraseFast(uint32_t addr, uint32_t size);

#ifdef __cplusplus
}
#endif


#endif
