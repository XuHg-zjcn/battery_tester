/*************************************************************************
 *  电池测试仪ADC采样
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
#include "stm32f1xx_ll_bus.h"
#include "stm32f1xx_ll_rcc.h"
#include "stm32f1xx_ll_gpio.h"
#include "stm32f1xx_ll_adc.h"
#include "stm32f1xx_ll_tim.h"
#include "stm32f1xx_ll_dma.h"
#include "conf.h"


#define ADC_DELAY_ENABLE_CALIB_CPU_CYCLES  (LL_ADC_DELAY_ENABLE_CALIB_ADC_CYCLES * 32)

uint16_t adc_dma_buff[32*2];


void ADC_Init()
{
  //Config GPIO
  LL_APB2_GRP1_EnableClock(APB2PER_ADDR2BIT(GPIO_PORT_ADC_CURR) | APB2PER_ADDR2BIT(GPIO_PORT_ADC_VOLT));
  LL_GPIO_SetPinMode(GPIO_PORT_ADC_CURR, LL_GPIO_PIN_ADC_CURR, LL_GPIO_MODE_ANALOG);
  LL_GPIO_SetPinMode(GPIO_PORT_ADC_VOLT, LL_GPIO_PIN_ADC_VOLT, LL_GPIO_MODE_ANALOG);

  //Config ADC
  LL_RCC_SetADCClockSource(LL_RCC_ADC_CLKSRC_PCLK2_DIV_6);
  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_ADC1);

  LL_ADC_REG_SetTriggerSource(ADC1, LL_ADC_REG_TRIG_EXT_TIM3_TRGO);
  LL_ADC_REG_SetContinuousMode(ADC1, LL_ADC_REG_CONV_SINGLE);
  LL_ADC_REG_SetSequencerDiscont(ADC1, LL_ADC_REG_SEQ_DISCONT_DISABLE);
  LL_ADC_SetSequencersScanMode(ADC1, LL_ADC_SEQ_SCAN_ENABLE);
  LL_ADC_REG_StartConversionExtTrig(ADC1, LL_ADC_REG_TRIG_EXT_RISING);

  //CH1: current, CH2: voltage
  LL_ADC_REG_SetSequencerLength(ADC1, LL_ADC_REG_SEQ_SCAN_ENABLE_2RANKS);
  LL_ADC_REG_SetSequencerRanks(ADC1, LL_ADC_REG_RANK_1, LL_ADC_CHANNEL_CURR);
  LL_ADC_REG_SetSequencerRanks(ADC1, LL_ADC_REG_RANK_2, LL_ADC_CHANNEL_VOLT);

  LL_ADC_SetChannelSamplingTime(ADC1, LL_ADC_CHANNEL_CURR, LL_ADC_SAMPLINGTIME_7CYCLES_5);
  LL_ADC_SetChannelSamplingTime(ADC1, LL_ADC_CHANNEL_VOLT, LL_ADC_SAMPLINGTIME_7CYCLES_5);

  LL_ADC_REG_SetDMATransfer(ADC1, LL_ADC_REG_DMA_TRANSFER_UNLIMITED);
  LL_ADC_Enable(ADC1);

  volatile uint32_t wait_loop_index = (ADC_DELAY_ENABLE_CALIB_CPU_CYCLES >> 1);
  while(wait_loop_index--);
  LL_ADC_StartCalibration(ADC1);
  while(LL_ADC_IsCalibrationOnGoing(ADC1) != 0);

  //Config DMA
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_DMA1);
  LL_DMA_ConfigTransfer(DMA1,
                        LL_DMA_CHANNEL_1,
                        LL_DMA_DIRECTION_PERIPH_TO_MEMORY |
                        LL_DMA_MODE_CIRCULAR              |
                        LL_DMA_PERIPH_NOINCREMENT         |
                        LL_DMA_MEMORY_INCREMENT           |
                        LL_DMA_PDATAALIGN_HALFWORD        |
                        LL_DMA_MDATAALIGN_HALFWORD        |
                        LL_DMA_PRIORITY_HIGH);
  LL_DMA_ConfigAddresses(DMA1, LL_DMA_CHANNEL_1,
		         LL_ADC_DMA_GetRegAddr(ADC1, LL_ADC_DMA_REG_REGULAR_DATA),
			 (uint32_t)adc_dma_buff,
			 LL_DMA_DIRECTION_PERIPH_TO_MEMORY);
  LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_1, 32*2);
  LL_DMA_EnableIT_TC(DMA1, LL_DMA_CHANNEL_1);
  LL_DMA_EnableIT_HT(DMA1, LL_DMA_CHANNEL_1);
  //LL_DMA_EnableIT_TE(DMA1, LL_DMA_CHANNEL_1);
  NVIC_SetPriority(DMA1_Channel1_IRQn, 0);
  NVIC_EnableIRQ(DMA1_Channel1_IRQn);

  //Config TIM
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM3);
  LL_TIM_SetPrescaler(TIM3, 0);
  LL_TIM_SetAutoReload(TIM3, 256-1);
  LL_TIM_SetCounterMode(TIM3, LL_TIM_COUNTERMODE_UP);
  LL_TIM_SetTriggerOutput(TIM3, LL_TIM_TRGO_UPDATE);

  //start
  LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_1);
  LL_TIM_EnableCounter(TIM3);
}
