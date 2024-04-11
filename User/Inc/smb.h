#ifndef SMB_H
#define SMB_H

#ifdef __cplusplus
extern "C"{
#endif

void SMB_Init();
void I2C_SMB_Write(uint16_t len, uint8_t i2c_addr, const uint8_t *data);
void I2C_SMB_Read(uint16_t tlen, uint16_t rlen, uint8_t i2c_addr, const uint8_t *data);

#ifdef __cplusplus
}
#endif

#endif
