/*************************************************************************
 *  电池测试仪FLASH相关
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
#include "stm32f1xx_hal_flash.h"
#include "usart.h"
#include "flash_ch32v2.h"

const uint32_t waddr0 = 0x08006100;
uint32_t waddr;

void Flash_Init()
{
  //TODO: 不应该初始化时解锁,而是需要写入时解锁,写入后上锁,减少误操作可能
  HAL_FLASH_Unlock();
  waddr = waddr0;
  while(*(uint32_t *)waddr != 0xe339e339 && waddr < 0x08010000){
    waddr += 256;
  }
  NVIC_SetPriority(FLASH_IRQn, 3);
  NVIC_EnableIRQ(FLASH_IRQn);
}

void Flash_Read(uint16_t raddr, uint8_t size)
{
  USART_Send(0x08000000UL+raddr, size);
}

void Flash_Erase()
{
  FLASH_WCH_EraseFast(waddr0, 0x08010000-waddr0);
}
