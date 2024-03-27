#include "oled.h"
#include "ssd1306.hpp"
#include "stm32f1xx_ll_bus.h"
#include "stm32f1xx_ll_gpio.h"
#include "c_i2c.hpp"

void OLED_Init()
{
  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_GPIOB);
  
  LL_GPIO_SetPinMode(GPIOB, LL_GPIO_PIN_10, LL_GPIO_MODE_ALTERNATE);
  LL_GPIO_SetPinOutputType(GPIOB, LL_GPIO_PIN_11, LL_GPIO_OUTPUT_OPENDRAIN);
  LL_GPIO_SetPinPull(GPIOB, LL_GPIO_PIN_10, LL_GPIO_PULL_UP);
  LL_GPIO_SetPinSpeed(GPIOB, LL_GPIO_PIN_10, LL_GPIO_SPEED_FREQ_LOW);
  
  LL_GPIO_SetPinMode(GPIOB, LL_GPIO_PIN_11, LL_GPIO_MODE_ALTERNATE);
  LL_GPIO_SetPinOutputType(GPIOB, LL_GPIO_PIN_11, LL_GPIO_OUTPUT_OPENDRAIN);
  LL_GPIO_SetPinPull(GPIOB, LL_GPIO_PIN_11, LL_GPIO_PULL_UP);
  LL_GPIO_SetPinSpeed(GPIOB, LL_GPIO_PIN_11, LL_GPIO_SPEED_FREQ_LOW);

  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_I2C2);

  C_I2C ci2c;
  ci2c.Instance             = I2C2;
  ci2c.Init.ClockSpeed      = 400000;
  ci2c.Init.DutyCycle       = I2C_DUTYCYCLE_16_9;
  ci2c.Init.OwnAddress1     = 0xFF;
  ci2c.Init.AddressingMode  = I2C_ADDRESSINGMODE_7BIT;
  ci2c.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  ci2c.Init.OwnAddress2     = 0xFF;
  ci2c.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  ci2c.Init.NoStretchMode   = I2C_NOSTRETCH_DISABLE;
  ci2c.set_Clock(400000);
  C_I2C_Dev dev = C_I2C_Dev(&ci2c, Addr_OLED, I2C_MEMADD_SIZE_8BIT);
  dev.set_TransMode(TransTypeStru({TransBlocking, false, false, false, Wait_error}));
  SSD1306 oled = SSD1306(&dev);
  oled.Init();
  oled.fill(0x00);

  oled.setVHAddr(Horz_Mode, 0, 127, 0, 0);
  oled.text_5x7("Battery tester");
  oled.setVHAddr(Horz_Mode, 0, 127, 2, 2);
  oled.text_5x7("this is a demo text!");
  oled.setVHAddr(Horz_Mode, 0, 127, 4, 4);
  oled.text_5x7("by Xu Ruijun");
}
