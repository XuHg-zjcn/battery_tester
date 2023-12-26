/*************************************************************************
 *  用于电池测试仪的串口驱动程序
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
#ifndef USART_H
#define USART_H

#include <stdint.h>


#define ARR_NELEM(x)           (sizeof(x)/sizeof(x[0]))

#define GPIO_PORT_USART        GPIOA
#define LL_GPIO_PIN_USART_TX   LL_GPIO_PIN_9
#define LL_GPIO_PIN_USART_RX   LL_GPIO_PIN_10

#define USART_PC               USART1
#define USART_PC_IRQn          USART1_IRQn

typedef struct{
  uint32_t addr;
  uint16_t size;
  uint16_t resv;
}DMAQueue_item;

void USART_Init();
void USART_Send(const uint8_t *data, uint32_t length);

#endif
