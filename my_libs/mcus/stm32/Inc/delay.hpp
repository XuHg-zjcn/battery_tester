/*
 * delay.h
 *
 *  Created on: 2021年4月16日
 *      Author: xrj
 */

#ifndef INC_DELAY_H_
#define INC_DELAY_H_

#include "mylibs_config.hpp"

#ifdef __STM32F1xx_HAL_H
#define clk_loop 13
#define clk_call 4
#define clk_getFreq 50
#endif
#ifdef __STM32F4xx_HAL_H
#define clk_loop 7
#define clk_call 3
#define clk_getFreq 37
#endif

void Delay_loopN(uint32_t n);
void Delay_clocks(uint32_t n);
void Delay_us(uint32_t us);

#endif /* INC_DELAY_H_ */
