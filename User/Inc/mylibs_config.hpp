#include "myints.h"
#include "x_base.hpp"
#include <stdlib.h>
#include "ch32v203_delay.h"
#include "stm32f1xx_hal_conf.h"

#define XMalloc(x) malloc(x)
#define XFree(x)   free(x)
#define XDelayMs(x) Delay_ms(x)
#define XDelayUs(x) Delay_us(x)

#define SSD1306_I2C_Dev C_I2C_Dev
