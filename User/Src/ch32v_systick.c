#include "ch32v_systick.h"
#include <stdint.h>

uint64_t SysTick_GetSafe()
{
  uint32_t h1 = SysTick_CH32V->CNTH;
  uint32_t l1 = SysTick_CH32V->CNTL;
  uint32_t h2 = SysTick_CH32V->CNTH;
  if(h1 == h2){
    return (((uint64_t)h1)<<32) | l1;
  }else{
    uint32_t l2 = SysTick_CH32V->CNTL;
    return (((uint64_t)h2)<<32) | l2;
  }
}
