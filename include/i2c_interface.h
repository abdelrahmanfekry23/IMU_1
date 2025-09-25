#ifndef I2C_INTERFACE_H
#define I2C_INTERFACE_H

/* -------- Minimal I2C API (polling, master) -------- */

/* Initialize clocks, GPIO, and timing (CR2/CCR/TRISE/CR1). */
void I2C1_Init(void);

/* Generate START and wait for SB flag. */
int  I2C1_Start(void);          /* return 0 on OK, <0 on timeout/error */

/* Send 7-bit address with R/W bit (read=1, write=0); clears ADDR properly. */
int  I2C1_SendAddress(unsigned int addr7, int read);  /* 0 OK, <0 err */

/* Write one byte to DR and (optionally) wait TXE/BTF. */
int  I2C1_WriteByte(unsigned int byte_val);           /* 0 OK, <0 err */

/* Read one byte with ACK=1 (for all but last byte). */
int  I2C1_ReadByte_ACK(unsigned int *out_byte);       /* 0 OK, <0 err */

/* Read last byte with ACK=0 and STOP sequencing (NACK). */
int  I2C1_ReadByte_NACK(unsigned int *out_byte);      /* 0 OK, <0 err */

/* Generate STOP. */
void I2C1_Stop(void);

/* Optional: bus wait/free and simple timeouts helper. */
int  I2C1_WaitBusFree(void);    /* 0 OK, <0 if BUSY timeout */

#endif /* I2C_INTERFACE_H */
