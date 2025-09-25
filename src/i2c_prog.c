/* prog.c */
#include "config.h"
#include "interface.h"


void I2C1_Init(void)
{
    unsigned int tmp;
    unsigned int freq_mhz;
    unsigned int ccr;

    /* 1) Enable clocks */
    SET_BITS(RCC_APB2ENR, RCC_APB2ENR_AFIOEN | RCC_APB2ENR_IOPBEN);
    SET_BITS(RCC_APB1ENR, RCC_APB1ENR_I2C1EN);

    /* 2) Use default mapping PB6/PB7 */
    CLR_BITS(AFIO_MAPR, AFIO_MAPR_I2C1_REMAP);

    /* 3) GPIO config: PB6/PB7 = AF Open-Drain @ 50MHz */
    GPIOB_CRL &= ~((GPIO_CRL_NIBBLE_MASK << GPIO_CRL_PB6_Pos) |
                   (GPIO_CRL_NIBBLE_MASK << GPIO_CRL_PB7_Pos));
    GPIOB_CRL |=  ((GPIO_MODE_AF_OD_50MHz << GPIO_CRL_PB6_Pos) |
                   (GPIO_MODE_AF_OD_50MHz << GPIO_CRL_PB7_Pos));

    /* 4) Clean reset */
    SET_BITS(I2C1_CR1, I2C_CR1_SWRST);
    CLR_BITS(I2C1_CR1, I2C_CR1_SWRST);

    /* 5) Disable peripheral while configuring */
    CLR_BITS(I2C1_CR1, I2C_CR1_PE);

    /* 6) CR2.FREQ = APB1 frequency in MHz (max 63) */
    freq_mhz = (PCLK1_HZ / 1000000U) & 0x3FU;
    tmp = I2C1_CR2 & ~I2C_CR2_FREQ_Msk;
    I2C1_CR2 = tmp | (freq_mhz << I2C_CR2_FREQ_Pos);

    /* 7) CCR/TRISE */
    if (I2C_SPEED_HZ <= 100000U)
    {
        /* Standard mode 100k: CCR = PCLK1/(2*speed), min 4 */
        ccr = PCLK1_HZ / (2U * I2C_SPEED_HZ);
        if (ccr < 4U) ccr = 4U;
        I2C1_CCR = ccr;              /* FS=0 */
        I2C1_TRISE = freq_mhz + 1U;  /* SM: TRISE = FREQ+1 */
    }
    else
    {
        /* Fast mode 400k (duty=2): CCR = PCLK1/(3*speed) */
        ccr = PCLK1_HZ / (3U * I2C_SPEED_HZ);
        if (ccr == 0U) ccr = 1U;
        I2C1_CCR = I2C_CCR_FS | ccr;                /* FS=1, DUTY=0 */
        I2C1_TRISE = ((freq_mhz * 3U) / 10U) + 1U;  /* ~300ns + 1 */
        if (I2C1_TRISE == 0U) I2C1_TRISE = 1U;
    }

    /* 8) ACK on by default */
    SET_BITS(I2C1_CR1, I2C_CR1_ACK);

    /* 9) Enable peripheral */
    SET_BITS(I2C1_CR1, I2C_CR1_PE);
}

/* ---- I2C1_WaitBusFree ----
   Wait until SR2.BUSY == 0
   Return: 0 OK, -1 timeout
*/
int I2C1_WaitBusFree(void)
{
    unsigned int t = I2C_TIMEOUT;
    while ((I2C1_SR2 & I2C_SR2_BUSY) && (--t)) { }
    return (t == 0U) ? -1 : 0;
}

/* ---- I2C1_Start ----
   Generate START and wait for SR1.SB
   Return: 0 OK, -1 timeout
*/
int I2C1_Start(void)
{
    unsigned int t;

    if (I2C1_WaitBusFree() != 0) return -1;

    SET_BITS(I2C1_CR1, I2C_CR1_START);

    t = I2C_TIMEOUT;
    while (!(I2C1_SR1 & I2C_SR1_SB) && (--t)) { }
    return (t == 0U) ? -1 : 0;
}

/* ---- I2C1_SendAddress ----
   Send 7-bit address + R/W
   Wait for SR1.ADDR, then clear by reading SR1 and SR2.
   Return: 0 OK, -1 timeout
*/
int I2C1_SendAddress(unsigned int addr7, int read)
{
    unsigned int t;

    I2C1_DR = ((addr7 & 0x7FU) << 1) | (read ? 1U : 0U);

    t = I2C_TIMEOUT;
    while (!(I2C1_SR1 & I2C_SR1_ADDR) && (--t)) { }
    if (t == 0U) return -1;

    (void)I2C1_SR1;
    (void)I2C1_SR2;
    return 0;
}

/* ---- I2C1_WriteByte ----
   Write one data byte and wait for TXE=1
   Return: 0 OK, -1 timeout
*/
int I2C1_WriteByte(unsigned int byte_val)
{
    unsigned int t;

    I2C1_DR = (byte_val & 0xFFU);

    t = I2C_TIMEOUT;
    while (!(I2C1_SR1 & I2C_SR1_TXE) && (--t)) { }
    return (t == 0U) ? -1 : 0;
}

/* ---- I2C1_ReadByte_ACK ----
   Ensure ACK=1, wait RXNE=1, read DR
   Return: 0 OK, -1 timeout
*/
int I2C1_ReadByte_ACK(unsigned int *out_byte)
{
    unsigned int t;
    if (!out_byte) return -1;

    SET_BITS(I2C1_CR1, I2C_CR1_ACK);

    t = I2C_TIMEOUT;
    while (!(I2C1_SR1 & I2C_SR1_RXNE) && (--t)) { }
    if (t == 0U) return -1;

    *out_byte = (I2C1_DR & 0xFFU);
    return 0;
}

/* ---- I2C1_ReadByte_NACK ----
   For the last byte: set ACK=0 (NACK), wait RXNE, issue STOP, read DR,
   then restore ACK=1.
   Return: 0 OK, -1 timeout
*/
int I2C1_ReadByte_NACK(unsigned int *out_byte)
{
    unsigned int t;
    if (!out_byte) return -1;

    CLR_BITS(I2C1_CR1, I2C_CR1_ACK);  /* NACK */

    t = I2C_TIMEOUT;
    while (!(I2C1_SR1 & I2C_SR1_RXNE) && (--t)) { }
    if (t == 0U) return -1;

    SET_BITS(I2C1_CR1, I2C_CR1_STOP); /* STOP for last byte */

    *out_byte = (I2C1_DR & 0xFFU);

    SET_BITS(I2C1_CR1, I2C_CR1_ACK);  /* restore ACK */
    return 0;
}

/* ---- I2C1_Stop ----
   Generate STOP (HW clears it after the condition).
*/
void I2C1_Stop(void)
{
    SET_BITS(I2C1_CR1, I2C_CR1_STOP);
}
