#include <stdio.h>
#include "pid.h"

PID_stat pid = {
  .p_smin = 10*1000,
  .p_smax = 10*4095,
  .p_emin = -500,
  .p_emax = 1000,
  .p_k = 0.1*(1UL<<32),

  .i_k = 0.1*(1UL<<32),

  .out_min = 1000,
  .out_max = 4096,
};

int main()
{
  PID_Init(&pid, -400, 4000);
  for(int i=0;i<100;i++){
  printf("%d,", PID_update(&pid, -400));
  }
  printf("\n");
  for(int i=0;i<100;i++){
  printf("%d,", PID_update(&pid, 400));
  }
  printf("\n");
}
