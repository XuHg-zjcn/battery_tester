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
#include "flash_ch32v2.h"

#define FLASH_ERASEFAST_IT_EN 0

typedef enum{
  FLASH_op_none = 0,
  FLASH_op_prog_norm = 1,
  FLASH_op_prog_fast = 2,
  FLASH_op_erase_norm = 3,
  FLASH_op_erase_fast = 4,
}FLASH_optype;

static FLASH_optype op;
static uint32_t flash_addr;
static uint64_t flash_tmp;
static uint32_t flash_remain;

uint8_t flash_buff[256];

#ifdef CH32V203x8
void FLASH_WCH_Unlock_Fast()
{
  WRITE_REG(FLASH_WCH->KEYR, KEY1);
  WRITE_REG(FLASH_WCH->KEYR, KEY2);
  WRITE_REG(FLASH_WCH->MODEKEYR, KEY1);
  WRITE_REG(FLASH_WCH->MODEKEYR, KEY2);
}

void FLASH_WCH_Lock_All()
{
  SET_BIT(FLASH_WCH->CTLR, FLASH_CTLR_FLOCK);
  SET_BIT(FLASH_WCH->CTLR, FLASH_CTLR_LOCK);
}

void FLASH_WCH_ProgramFast_Init()
{
  if(op != FLASH_op_none){
    return;
  }
  op = FLASH_op_prog_fast;
  while(FLASH_WCH_IsBusy());
  SET_BIT(FLASH_WCH->CTLR, FLASH_CTLR_FTPG);
}

void FLASH_WCH_ProgramFast_BuffLoad4B(uint32_t addr, uint32_t data)
{
  while(READ_BIT(FLASH_WCH->STATR, FLASH_STATR_WR_BUSY));
  *(uint32_t *)(addr) = data;
}

void FLASH_WCH_ProgramFast_start()
{
  while(READ_BIT(FLASH_WCH->STATR, FLASH_STATR_WR_BUSY));
  SET_BIT(FLASH_WCH->CTLR, FLASH_CTLR_PGSTRT);
  while(FLASH_WCH_IsBusy());
  CLEAR_BIT(FLASH_WCH->CTLR, FLASH_CTLR_FTPG);
}

void FLASH_WCH_EraseFast(uint32_t addr, uint32_t size)
{
  if(addr&0xff != 0){
    return;
  }
  if(addr < 0x08000000 || addr >= 0x08010000){
    return;
  }
  op = FLASH_op_erase_fast;
  while(FLASH_WCH_IsBusy());
  SET_BIT(FLASH_WCH->CTLR, FLASH_CTLR_FTER);
#if FLASH_ERASEFAST_IT_EN
  WRITE_REG(FLASH_WCH->ADDR, addr);
  SET_BIT(FLASH_WCH->CTLR, FLASH_CTLR_STRT);
  if(size > 256){
    flash_addr = addr + 256;
    flash_remain = size - 256;
  }
#else
  while(size > 0){
    WRITE_REG(FLASH_WCH->ADDR, addr);
    SET_BIT(FLASH_WCH->CTLR, FLASH_CTLR_STRT);
    while(FLASH_WCH_IsBusy());
    SET_BIT(FLASH_WCH->STATR, FLASH_STATR_EOP);
    addr += 256;
    size -= 256;
  }
  CLEAR_BIT(FLASH_WCH->CTLR, FLASH_CTLR_FTER);
  op = FLASH_op_none;
#endif
}

void FLASH_Program(uint32_t addr, uint64_t data, uint32_t size)
{
  if(op != FLASH_op_none){
    return;
  }
  op = FLASH_op_prog_norm;
  while(FLASH_WCH_IsBusy());
  SET_BIT(FLASH_WCH->CTLR, FLASH_CTLR_PG);
  *(uint16_t *)(addr) = data&0xffff;
  if(size > 2){
    flash_addr = addr+2;
    flash_tmp = data>>16;
    flash_remain = size-2;
  }
}

void MyFLASH_IRQHandler()
{
  if(READ_BIT(FLASH_WCH->STATR, FLASH_STATR_EOP)){
    SET_BIT(FLASH_WCH->STATR, FLASH_STATR_EOP);
    if(READ_BIT(FLASH_WCH->CTLR, FLASH_CTLR_PG) && op == FLASH_op_prog_norm){
      if(flash_remain > 0){
	*(uint16_t *)(flash_addr) = flash_tmp&0xffff;
	flash_remain -= 2;
	flash_addr += 2;
	flash_tmp >>= 16;
      }
      if(flash_remain <= 0){
	CLEAR_BIT(FLASH_WCH->CTLR, FLASH_CTLR_PG);
	op = FLASH_op_none;
      }
    }
#if FLASH_PROGFAST_IT_EN
    if(READ_BIT(FLASH_WCH->CTLR, FLASH_CTLR_FTER) && op == FLASH_op_erase_fast){
      if(flash_remain > 0){
	WRITE_REG(FLASH_WCH->ADDR, flash_addr);
	SET_BIT(FLASH_WCH->CTLR, FLASH_CTLR_STRT);
	flash_remain -= 256;
	flash_addr += 256;
      }
      if(flash_remain <= 0){
	CLEAR_BIT(FLASH_WCH->CTLR, FLASH_CTLR_FTER);
	op = FLASH_op_none;
      }
    }
#endif
  }
}
#endif
