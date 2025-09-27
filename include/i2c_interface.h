#ifndef I2C_INTERFACE_H
#define I2C_INTERFACE_H

#include "i2c_config.h"

void          I2C1_Init(void);
void          I2C1_Start(void);
int           I2C1_Address(unsigned char addr7, int dir /*0=w,1=r*/);
int           I2C1_WriteByte(unsigned char data);
unsigned char I2C1_ReadAck(void);
unsigned char I2C1_ReadNack(void);
void          I2C1_Stop(void);

/* Small helpers */
int           I2C1_Probe(unsigned char addr7);
int           I2C1_ReadReg1(unsigned char addr7, unsigned char reg, unsigned char *val);

#endif
