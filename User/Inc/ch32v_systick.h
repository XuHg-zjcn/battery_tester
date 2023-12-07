#ifndef CH32V_SYSTICK_H
#define CH32V_SYSTICK_H

#include <stdint.h>

typedef struct{
  volatile uint32_t CTLR;
  volatile uint32_t SR;
  volatile uint32_t CNTL;
  volatile uint32_t CNTH;
  volatile uint32_t CMPLR;
  volatile uint32_t CMPHR;
}SysTick_CH32V_TypeDef;

#define SysTick_CH32V_BASE  (0xE000F000)
#define SysTick_CH32V       ((SysTick_CH32V_TypeDef *)SysTick_CH32V_BASE)

uint64_t SysTick_GetSafe();

#endif
