/*************************************************************************
 *  电池测试仪配置文件
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
#ifndef CONF_H
#define CONF_H

#include "ch32v203x8.h"
#include "ops.h"

#define FLASH_DATAWRITE       1

//#define MIN(a, b) (((a)<(b))?(a):(b))

//ADC电流和电压采样
#define GPIO_PORT_ADC_CURR    GPIOA
#define LL_GPIO_PIN_ADC_CURR  LL_GPIO_PIN_1
#define LL_ADC_CHANNEL_CURR   LL_ADC_CHANNEL_1
#define GPIO_PORT_ADC_VOLT    GPIOA
#define LL_GPIO_PIN_ADC_VOLT  LL_GPIO_PIN_2
#define LL_ADC_CHANNEL_VOLT   LL_ADC_CHANNEL_2

//用于控制MOS管的PWM输出
#define GPIO_PORT_PWM         GPIOA
#define LL_GPIO_PIN_PWM       LL_GPIO_PIN_15
#define TIMx_PWM               TIM2
#define NAMECONN_PWM_TIMx(x)   x##2
#define LL_TIM_CHANNEL_CHx_PWM LL_TIM_CHANNEL_CH1
#define NAMECONN_PWM_CHx(x)    x##1

//显示屏I2C
#define I2Cx_OLED             I2C2
#define I2C_CLOCKSPEED_OLED   400000
#define GPIO_PORT_OLED_SCL    GPIOB
#define LL_GPIO_PIN_OLED_SCL  LL_GPIO_PIN_10
#define GPIO_PORT_OLED_SDA    GPIOB
#define LL_GPIO_PIN_OLED_SDA  LL_GPIO_PIN_11

//与BMS系统通信的I2C SMBus
#define I2Cx_SMB              I2C1
#define I2C_CLOCKSPEED_SMB    100000
#define I2C_DUTYCYCLE_SMB     LL_I2C_DUTYCYCLE_2
#define GPIO_PORT_SMB_SCL     GPIOB
#define LL_GPIO_PIN_SMB_SCL   LL_GPIO_PIN_6
#define GPIO_PORT_SMB_SDA     GPIOB
#define LL_GPIO_PIN_SMB_SDA   LL_GPIO_PIN_7
#define LL_DMA_CHANNELx_I2C_SMB_TX      LL_DMA_CHANNEL_6
#define LL_DMA_CHANNELx_I2C_SMB_RX      LL_DMA_CHANNEL_7
#define DMA1_Channelx_I2C_SMB_TX_IRQn   DMA1_Channel6_IRQn
#define DMA1_Channelx_I2C_SMB_RX_IRQn   DMA1_Channel7_IRQn
#define I2Cx_SMB_EV_IRQn                I2C1_EV_IRQn
#define I2Cx_SMB_ER_IRQn                I2C1_ER_IRQn

//与电脑通信的串口
#define USARTx_PC             USART1
#define USART_BAUD            115200
#define GPIO_PORT_USART_TX    GPIOA
#define LL_GPIO_PIN_USART_TX  LL_GPIO_PIN_9
#define GPIO_PORT_USART_RX    GPIOA
#define LL_GPIO_PIN_USART_RX  LL_GPIO_PIN_10
#define USARTx_PC_IRQn        USART1_IRQn
#define LL_DMA_CHANNELx_USART_PC_TX     LL_DMA_CHANNEL_4
#define DMA1_Channelx_USART_PC_TX_IRQn  DMA1_Channel4_IRQn

//仍然需要手动修改源代码 引脚Remap, 中断函数, ADC触发定时器
#define IS_APB1PERIPH_ADDR(x)   (APB1PERIPH_BASE<=((uint32_t)(x)) && ((uint32_t)(x))<(APB1PERIPH_BASE + 32*0x400))
#define IS_APB2PERIPH_ADDR(x)   (APB2PERIPH_BASE<=((uint32_t)(x)) && ((uint32_t)(x))<(APB2PERIPH_BASE + 32*0x400))
#define APB1PER_ADDR2BIT(x)     (1U<<MIN((((uint32_t)(x))-APB1PERIPH_BASE)/0x400, 31))
#define APB2PER_ADDR2BIT(x)     (1U<<MIN((((uint32_t)(x))-APB2PERIPH_BASE)/0x400, 31))
#define ENABLE_CLOCK_BY_ADDR(x) do{\
                                  if(IS_APB1PERIPH_ADDR(x)){LL_APB1_GRP1_EnableClock(APB1PER_ADDR2BIT(x));}else \
	                          if(IS_APB2PERIPH_ADDR(x)){LL_APB2_GRP1_EnableClock(APB2PER_ADDR2BIT(x));} \
				}while(0)

#endif
