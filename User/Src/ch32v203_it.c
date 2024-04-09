/*************************************************************************
 *  电池测试仪中断函数文件
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
#include "ch32v203_it.h"
#include "stm32f1xx_ll_exti.h"
#include "stm32f1xx_ll_gpio.h"
#include "stm32f1xx_ll_dma.h"
#include "stm32f1xx_ll_usart.h"
#include "ch32v_systick.h"
#include "usart.h"
#include "pid.h"
#include "mos_pwm.h"
#include "command.h"
#include "calib.h"
#include "ops64.h"
#include "func32.h"
#include <stddef.h>
#include "conf.h"


void DMA1_Channel1_IRQHandler(void) __attribute__((interrupt()));
void USART1_IRQHandler(void) __attribute__((interrupt()));
void DMA1_Channel4_IRQHandler(void) __attribute__((interrupt()));

extern uint16_t adc_dma_buff[32*2];
extern PID_stat pid;

uint32_t update_count = 0;
static uint32_t sumI_, sumU_ = 0;
int64_t sumQ, sumE = 0;
uint32_t sumI256, sumU256;
static uint16_t usart_data[3];

static const uint8_t report_stop[6] = {0xff, 0xfe, 'S', 'T', 'O', 'P'};
static const uint8_t cmdhead[4] = {0xAA, 'C', 'M', 'D'};
static uint64_t usart_rx_last_ts = 0; //接收到最后一个字节的时间戳
static uint32_t cmd_i = 0;
static uint8_t rxbuff[32];
extern uint32_t wave_logfcurr;
extern int32_t wave_phase;
extern DMAQueue_item usart_txqueue[4];
extern volatile int32_t usart_tx_begin; //下一个发送任务,-1表示空
extern volatile int32_t usart_tx_end; //任务尾部+1

void DMA1_Channel1_IRQHandler(void)
{
  const uint16_t *p = NULL;
  if(LL_DMA_IsActiveFlag_HT1(DMA1)){
    LL_DMA_ClearFlag_HT1(DMA1);
    p = &adc_dma_buff[0];
  }
  if(LL_DMA_IsActiveFlag_TC1(DMA1)){
    LL_DMA_ClearFlag_TC1(DMA1);
    p = &adc_dma_buff[32];
  }
  if(p){
    uint32_t sumI = 0;
    uint32_t sumU = 0;
    for(int i=0;i<16;i++){
      sumI += p[i*2+0];
      sumU += p[i*2+1];
    }
    if(sumU < stop_vmin && mode != Mode_Stop){
      mode = Mode_Stop;
      USART_Send(report_stop, sizeof(report_stop));
    }
    if(mode == Mode_ConsCurr){
      MOS_Set(PID_update(&pid, (int32_t)sumI-curr));
    }else if(mode == Mode_CurrWave){
      int32_t wave_delta = I32xU32_HI32(sin32(wave_phase), wave_amp);
      MOS_Set(PID_update(&pid, (int32_t)sumI-((int32_t)curr+wave_delta)));
      wave_phase += exp32x(wave_logfcurr);
      wave_logfcurr += wave_logdfdt;
      if((wave_logdfdt>0 && (wave_logfcurr>>16)>=wave_logfmax) ||
         (wave_logdfdt<0 && (wave_logfcurr>>16)<wave_logfmin)){
	mode = Mode_Stop;
        USART_Send(report_stop, sizeof(report_stop));
      }
    }else{
      MOS_Set(4096);
    }
    update_count++;
    sumI_ += sumI;
    sumU_ += sumU;
    if(update_count%16 == 0){
      usart_data[0] = 0xffff;
      usart_data[1] = sumI_/16;
      usart_data[2] = sumU_/16;
      sumI256 = sumI_;
      sumU256 = sumU_;
      if(mode != Mode_Stop){
        int32_t sumI_noOffset = sumI_to_noOffset(sumI256);
        int32_t sumU_noOffset = sumU_to_noOffset(sumU256, sumI256);
        sumQ += sumI_noOffset;
        sumE += ((int64_t)sumU_noOffset*sumI_noOffset)>>16;
      }
      sumI_ = 0;
      sumU_ = 0;
      USART_Send((uint8_t *)usart_data, sizeof(uint16_t)*3);
    }
  }
}

void USART1_IRQHandler(void)
{
  if(LL_USART_IsActiveFlag_RXNE(USARTx_PC)){
    uint8_t byte = LL_USART_ReceiveData8(USARTx_PC);
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
	rxbuff[cmd_i-4] = byte;
	if(cmd_i-4 >= rxbuff[0]){
	  cmd_i = 0;
	  ExecCmd(rxbuff);
	}else{
	  cmd_i++;
	}
      }
    }
    usart_rx_last_ts = now;
  }
}

void DMA1_Channel4_IRQHandler(void)
{
  if(LL_DMA_IsActiveFlag_TC4(DMA1)){
    LL_DMA_ClearFlag_TC4(DMA1);
    LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_4);
    int i = usart_tx_begin;
    if(i >= 0){
      LL_DMA_SetMemoryAddress(DMA1, LL_DMA_CHANNEL_4, usart_txqueue[i].addr);
      LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_4, usart_txqueue[i].size);
      LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_4);
      usart_txqueue[i].addr = 0;
      int32_t next = (i+1)%ARR_NELEM(usart_txqueue);
      if(next != usart_tx_end){
	usart_tx_begin = next;
      }else{
	usart_tx_begin = -1;
	//已经是队列中最后一个了
	//但在这次的发送过程中又添加新消息，可以直接覆盖usart_txqueue[0]
      }
    }
  }
}
