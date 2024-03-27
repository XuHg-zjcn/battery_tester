/*
 * c_i2c.hpp
 *
 *  Created on: May 19, 2021
 *      Author: xrj
 */

#include "mylibs_config.hpp"
#if !defined(STM32_INC_C_I2C_HPP_) && defined(HAL_I2C_MODULE_ENABLED)
#define STM32_INC_C_I2C_HPP_

#include "c_stream.hpp"
#include "x_base.hpp"

class C_I2C : public I2C_HandleTypeDef{
public:
	X_State set_Clock(uint32_t Hz);
    X_State send(uint16_t DevAddress, uint8_t* pData, uint32_t Size);
    X_State recv(uint16_t DevAddress, uint8_t* pData, uint32_t Size);
};


#ifdef USE_ABSTRACT
class C_I2C_Dev : public Stream{
#else
class C_I2C_Dev{
#endif
private:
	C_I2C *hi2c;
	const uint16_t DevAddr;      //address of I2C device
	const uint16_t MemAdd_size;  //memory addr size for mem write/mem read
    uint32_t ClockHz;      //reconfig I2C freq before each communication, 0: don't reconfig
    TransTypeStru trans;
    uint32_t Timeout;
public:
    C_I2C_Dev(C_I2C *hi2c, uint16_t addr, uint16_t mem_size);
    void set_Clock(uint32_t Hz);
    void set_Timeout(uint32_t ms);
    void set_TransMode(TransTypeStru trans);
    X_State send(uint8_t* data, uint32_t size);
    X_State recv(uint8_t* data, uint32_t size);
    X_State Mem_write(uint16_t mem_addr, uint8_t *pData, uint16_t Size);
    X_State Mem_read(uint16_t mem_addr, uint8_t *pData, uint16_t Size);
};


#endif /* STM32_INC_C_I2C_HPP_ */
