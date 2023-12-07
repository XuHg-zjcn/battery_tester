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
#include "usart.h"
#include "stm32f1xx_ll_bus.h"
#include "stm32f1xx_ll_gpio.h"
#include "stm32f1xx_ll_usart.h"
#include "ch32v_systick.h"
#include "command.h"

#define GPIO_PORT_USART        GPIOA
#define LL_GPIO_PIN_USART_TX   LL_GPIO_PIN_9
#define LL_GPIO_PIN_USART_RX   LL_GPIO_PIN_10

#define USART_PC               USART1
#define USART_PC_IRQn          53

static const uint8_t cmdhead[4] = {0xAA, 'C', 'M', 'D'};
static uint64_t usart_rx_last_ts = 0; //接收到最后一个字节的时间戳
static uint32_t cmd_i = 0;

uint8_t buff[32];

void USART1_IRQHandler(void) __attribute__((interrupt()));

void USART_Init()
{
  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_GPIOA);
  LL_GPIO_SetPinMode(GPIO_PORT_USART, LL_GPIO_PIN_USART_TX, LL_GPIO_MODE_ALTERNATE);
  LL_GPIO_SetPinSpeed(GPIO_PORT_USART, LL_GPIO_PIN_USART_TX, LL_GPIO_SPEED_FREQ_MEDIUM);
  LL_GPIO_SetPinOutputType(GPIO_PORT_USART, LL_GPIO_PIN_USART_TX, LL_GPIO_OUTPUT_PUSHPULL);

  LL_GPIO_SetPinMode(GPIO_PORT_USART, LL_GPIO_PIN_USART_RX, LL_GPIO_MODE_INPUT);
  LL_GPIO_SetPinPull(GPIO_PORT_USART, LL_GPIO_PIN_USART_RX, LL_GPIO_PULL_UP);

  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_USART1);
  LL_USART_SetTransferDirection(USART_PC, LL_USART_DIRECTION_TX_RX);
  LL_USART_ConfigCharacter(USART_PC, LL_USART_DATAWIDTH_8B, LL_USART_PARITY_NONE, LL_USART_STOPBITS_1);
  LL_USART_SetBaudRate(USART_PC, SystemCoreClock, 115200);

  NVIC_EnableIRQ(USART_PC_IRQn);
  NVIC_SetPriority(USART_PC_IRQn, 1);

  LL_USART_EnableIT_RXNE(USART_PC);
  LL_USART_Enable(USART_PC);
}

void USART_Send(const uint8_t *data, uint32_t length)
{
  while(length--){
    while(!LL_USART_IsActiveFlag_TXE(USART_PC));
    LL_USART_TransmitData8(USART_PC, *data++);
  }
}

void USART1_IRQHandler(void)
{
  if(LL_USART_IsActiveFlag_RXNE(USART_PC)){
    uint8_t byte = LL_USART_ReceiveData8(USART_PC);
    uint64_t now = SysTick_GetSafe();
    if(cmd_i < 0 && now - usart_rx_last_ts > 72/8*5000){
      cmd_i = 0;
    }
    if(cmd_i >= 0){
      if(cmd_i < 4){
	if(byte == cmdhead[cmd_i]){
	  cmd_i++;
	}else{
	  //不匹配，标记为错误
	  cmd_i = 0;
	}
      }else{
	buff[cmd_i-4] = byte;
	if(cmd_i-4 >= buff[0]){
	  cmd_i = 0;
	  ExecCmd(buff);
	}else{
	  cmd_i++;
	}
      }
    }
    usart_rx_last_ts = now;
  }
}
