/*************************************************************************
 *  电池测试仪SMBus驱动
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
#include "stm32f1xx_ll_bus.h"
#include "stm32f1xx_ll_gpio.h"
#include "stm32f1xx_ll_i2c.h"
#include "stm32f1xx_ll_dma.h"
#include "conf.h"

uint8_t smb_buff[256];
int16_t smb_rxbyte=0;
uint8_t smb_addr;

void SMB_Init()
{
  LL_I2C_InitTypeDef I2C_InitStruct;

  LL_APB2_GRP1_EnableClock(APB2PER_ADDR2BIT(GPIO_PORT_SMB_SCL) | APB2PER_ADDR2BIT(GPIO_PORT_SMB_SDA));

  LL_GPIO_SetPinMode(GPIO_PORT_SMB_SCL, LL_GPIO_PIN_SMB_SCL, LL_GPIO_MODE_ALTERNATE);
  LL_GPIO_SetPinOutputType(GPIO_PORT_SMB_SCL, LL_GPIO_PIN_SMB_SCL, LL_GPIO_OUTPUT_OPENDRAIN);
  LL_GPIO_SetPinSpeed(GPIO_PORT_SMB_SCL, LL_GPIO_PIN_SMB_SCL, LL_GPIO_SPEED_FREQ_LOW);
  LL_GPIO_SetPinPull(GPIO_PORT_SMB_SCL, LL_GPIO_PIN_SMB_SCL, LL_GPIO_PULL_UP);

  LL_GPIO_SetPinMode(GPIO_PORT_SMB_SDA, LL_GPIO_PIN_SMB_SDA, LL_GPIO_MODE_ALTERNATE);
  LL_GPIO_SetPinOutputType(GPIO_PORT_SMB_SDA, LL_GPIO_PIN_SMB_SDA, LL_GPIO_OUTPUT_OPENDRAIN);
  LL_GPIO_SetPinSpeed(GPIO_PORT_SMB_SDA, LL_GPIO_PIN_SMB_SDA, LL_GPIO_SPEED_FREQ_LOW);
  LL_GPIO_SetPinPull(GPIO_PORT_SMB_SDA, LL_GPIO_PIN_SMB_SDA, LL_GPIO_PULL_UP);

  ENABLE_CLOCK_BY_ADDR(I2Cx_SMB);
  I2C_InitStruct.PeripheralMode  = LL_I2C_MODE_SMBUS_HOST;
  I2C_InitStruct.ClockSpeed      = I2C_CLOCKSPEED_SMB;
  I2C_InitStruct.DutyCycle       = I2C_DUTYCYCLE_SMB;
  I2C_InitStruct.OwnAddress1     = 0x00;
  I2C_InitStruct.TypeAcknowledge = LL_I2C_ACK;
  I2C_InitStruct.OwnAddrSize     = LL_I2C_OWNADDRESS1_7BIT;
  LL_I2C_Init(I2Cx_SMB, &I2C_InitStruct);
  //LL_I2C_EnableSMBusPEC(I2Cx_SMB);

  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_DMA1);
  LL_DMA_ConfigTransfer(DMA1,
                        LL_DMA_CHANNELx_I2C_SMB_TX,
                        LL_DMA_DIRECTION_MEMORY_TO_PERIPH |
                        LL_DMA_MODE_NORMAL                |
                        LL_DMA_PERIPH_NOINCREMENT         |
                        LL_DMA_MEMORY_INCREMENT           |
                        LL_DMA_PDATAALIGN_BYTE            |
                        LL_DMA_MDATAALIGN_BYTE            |
                        LL_DMA_PRIORITY_MEDIUM);
  LL_DMA_ConfigTransfer(DMA1,
                        LL_DMA_CHANNELx_I2C_SMB_RX,
                        LL_DMA_DIRECTION_PERIPH_TO_MEMORY |
                        LL_DMA_MODE_NORMAL                |
                        LL_DMA_PERIPH_NOINCREMENT         |
                        LL_DMA_MEMORY_INCREMENT           |
                        LL_DMA_PDATAALIGN_BYTE            |
                        LL_DMA_MDATAALIGN_BYTE            |
                        LL_DMA_PRIORITY_MEDIUM);
  LL_DMA_SetPeriphAddress(DMA1, LL_DMA_CHANNELx_I2C_SMB_TX, LL_I2C_DMA_GetRegAddr(I2Cx_SMB));
  LL_DMA_SetPeriphAddress(DMA1, LL_DMA_CHANNELx_I2C_SMB_RX, LL_I2C_DMA_GetRegAddr(I2Cx_SMB));
  LL_I2C_Enable(I2Cx_SMB);

  NVIC_SetPriority(I2Cx_SMB_EV_IRQn, 2);
  NVIC_EnableIRQ(I2Cx_SMB_EV_IRQn);
  NVIC_SetPriority(I2Cx_SMB_ER_IRQn, 2);
  NVIC_EnableIRQ(I2Cx_SMB_ER_IRQn);
  LL_I2C_EnableIT_EVT(I2Cx_SMB);
  LL_I2C_EnableIT_ERR(I2Cx_SMB);

  NVIC_SetPriority(DMA1_Channelx_I2C_SMB_TX_IRQn, 2);
  NVIC_EnableIRQ(DMA1_Channelx_I2C_SMB_TX_IRQn);
  LL_DMA_EnableIT_TC(DMA1, LL_DMA_CHANNELx_I2C_SMB_TX);
  LL_I2C_EnableDMAReq_TX(I2Cx_SMB);

  NVIC_SetPriority(DMA1_Channelx_I2C_SMB_RX_IRQn, 2);
  NVIC_EnableIRQ(DMA1_Channelx_I2C_SMB_RX_IRQn);
  LL_DMA_EnableIT_TC(DMA1, LL_DMA_CHANNELx_I2C_SMB_RX);
  LL_I2C_EnableDMAReq_RX(I2Cx_SMB);
}

//TODO: 添加PEC支持
//len不包含I2C设备地址
void I2C_SMB_Write(uint16_t len, uint8_t i2c_addr, const uint8_t *data)
{
  if(LL_DMA_IsEnabledChannel(DMA1, LL_DMA_CHANNELx_I2C_SMB_TX) || \
     LL_DMA_IsEnabledChannel(DMA1, LL_DMA_CHANNELx_I2C_SMB_RX)){
    return;
  }
  smb_rxbyte = 0;
  smb_addr = i2c_addr;
  LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNELx_I2C_SMB_TX);
  LL_DMA_SetMemoryAddress(DMA1, LL_DMA_CHANNELx_I2C_SMB_TX, (uint32_t)data);
  //DMA传输结束后立即设置STOP最后一字节无法发送，故DMA多传输一个无用字节
  LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNELx_I2C_SMB_TX, len+1);
  LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNELx_I2C_SMB_TX);
  LL_I2C_AcknowledgeNextData(I2Cx_SMB, LL_I2C_ACK);
  LL_I2C_GenerateStartCondition(I2Cx_SMB);
}

//tlen不包含I2C设备地址
void I2C_SMB_Read(uint16_t tlen, uint16_t rlen, uint8_t i2c_addr, const uint8_t *data)
{
  //TODO: 网上看到接收少于2字节时DMA不发出EOT_1, 设置了LAST位也无法发送NACK结束, 这时应该不要使用DMA
  if(LL_DMA_IsEnabledChannel(DMA1, LL_DMA_CHANNELx_I2C_SMB_TX) || \
     LL_DMA_IsEnabledChannel(DMA1, LL_DMA_CHANNELx_I2C_SMB_RX)){
    return;
  }
  smb_rxbyte = rlen;
  smb_addr = i2c_addr;
  LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNELx_I2C_SMB_TX);
  LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNELx_I2C_SMB_RX);
  LL_DMA_SetMemoryAddress(DMA1, LL_DMA_CHANNELx_I2C_SMB_TX, (uint32_t)data);
  LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNELx_I2C_SMB_TX, tlen+1);
  LL_DMA_SetMemoryAddress(DMA1, LL_DMA_CHANNELx_I2C_SMB_RX, (uint32_t)smb_buff);
  LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNELx_I2C_SMB_RX, rlen);
  LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNELx_I2C_SMB_RX);
  LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNELx_I2C_SMB_TX);
  LL_I2C_AcknowledgeNextData(I2Cx_SMB, LL_I2C_ACK);
  LL_I2C_GenerateStartCondition(I2Cx_SMB);
}
