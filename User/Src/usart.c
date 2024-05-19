/*************************************************************************
 *  用于电池测试仪的串口驱动程序
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
#include "usart.h"
#include "stm32f1xx_ll_bus.h"
#include "stm32f1xx_ll_gpio.h"
#include "stm32f1xx_ll_usart.h"
#include "stm32f1xx_ll_dma.h"
#include "conf.h"

#if USART_PC_EN
DMAQueue_item usart_txqueue[4];
volatile int32_t usart_tx_begin = -1; //下一个发送任务,-1表示空
volatile int32_t usart_tx_end = 0; //任务尾部+1

void USART_Init()
{
  LL_APB2_GRP1_EnableClock(APB2PER_ADDR2BIT(GPIO_PORT_USART_TX) | APB2PER_ADDR2BIT(GPIO_PORT_USART_RX));
  LL_GPIO_SetPinMode(GPIO_PORT_USART_TX, LL_GPIO_PIN_USART_TX, LL_GPIO_MODE_ALTERNATE);
  LL_GPIO_SetPinSpeed(GPIO_PORT_USART_TX, LL_GPIO_PIN_USART_TX, LL_GPIO_SPEED_FREQ_MEDIUM);
  LL_GPIO_SetPinOutputType(GPIO_PORT_USART_TX, LL_GPIO_PIN_USART_TX, LL_GPIO_OUTPUT_PUSHPULL);

  LL_GPIO_SetPinMode(GPIO_PORT_USART_RX, LL_GPIO_PIN_USART_RX, LL_GPIO_MODE_INPUT);
  LL_GPIO_SetPinPull(GPIO_PORT_USART_RX, LL_GPIO_PIN_USART_RX, LL_GPIO_PULL_UP);

  ENABLE_CLOCK_BY_ADDR(USARTx_PC);
  LL_USART_SetTransferDirection(USARTx_PC, LL_USART_DIRECTION_TX_RX);
  LL_USART_ConfigCharacter(USARTx_PC, LL_USART_DATAWIDTH_8B, LL_USART_PARITY_NONE, LL_USART_STOPBITS_1);
  LL_USART_SetBaudRate(USARTx_PC, SystemCoreClock, USART_BAUD);
  LL_USART_EnableDMAReq_TX(USARTx_PC);

  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_DMA1);
  LL_DMA_ConfigTransfer(DMA1,
                        LL_DMA_CHANNELx_USART_PC_TX,
                        LL_DMA_DIRECTION_MEMORY_TO_PERIPH |
                        LL_DMA_MODE_CIRCULAR              |
                        LL_DMA_PERIPH_NOINCREMENT         |
                        LL_DMA_MEMORY_INCREMENT           |
                        LL_DMA_PDATAALIGN_BYTE            |
                        LL_DMA_MDATAALIGN_BYTE            |
                        LL_DMA_PRIORITY_MEDIUM);
  LL_DMA_SetPeriphAddress(DMA1, LL_DMA_CHANNELx_USART_PC_TX, LL_USART_DMA_GetRegAddr(USARTx_PC));
  NVIC_SetPriority(DMA1_Channelx_USART_PC_TX_IRQn, 0);
  LL_DMA_EnableIT_TC(DMA1, LL_DMA_CHANNELx_USART_PC_TX);

  NVIC_SetPriority(DMA1_Channelx_USART_PC_TX_IRQn, 1);
  NVIC_SetPriority(USARTx_PC_IRQn, 1);
  NVIC_EnableIRQ(DMA1_Channelx_USART_PC_TX_IRQn);
  NVIC_EnableIRQ(USARTx_PC_IRQn);

  LL_USART_EnableIT_RXNE(USARTx_PC);
  LL_USART_Enable(USARTx_PC);
}

void USART_Send(const uint8_t *data, uint32_t length)
{
  NVIC_DisableIRQ(DMA1_Channelx_USART_PC_TX_IRQn);
  if(!LL_DMA_IsEnabledChannel(DMA1, LL_DMA_CHANNELx_USART_PC_TX) && usart_tx_begin == -1){
    //DMA通道关闭，并且不是在地址更新过程中，可以将传入地址直接写入DMA控制器
    LL_DMA_SetMemoryAddress(DMA1, LL_DMA_CHANNELx_USART_PC_TX, (uint32_t)&data[0]);
    LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNELx_USART_PC_TX, length);
    LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNELx_USART_PC_TX);
  }else{
    while(usart_tx_begin == usart_tx_end && usart_tx_begin>=0); //等待环形队列至少有一个空位
    int i; //传入数据指针存放位置
    if(usart_tx_begin < 0){
      //当前队列为空，添加到0处
      usart_tx_begin = 0;
      i = 0;
    }else{
      //添加到队列尾部
      i = usart_tx_end;
    }
    usart_txqueue[i].addr = (uint32_t)data;
    usart_txqueue[i].size = length;
    usart_tx_end = (i+1)%ARR_NELEM(usart_txqueue); //计算新末尾
  }
  NVIC_EnableIRQ(DMA1_Channelx_USART_PC_TX_IRQn);
}

#endif
