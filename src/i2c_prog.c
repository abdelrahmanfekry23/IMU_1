/* prog.c */
#include "i2c_config.h"
#include "i2c_interface.h"

static void tiny_delay(volatile unsigned int t){ while(t--){ __asm__ volatile("nop"); } }

static void i2c1_timing_stdmode(void)
{
    unsigned int freq_mhz = (PCLK1_HZ + 999999U)/1000000U;
    if (freq_mhz < 2U)  freq_mhz = 2U;
    if (freq_mhz > 36U) freq_mhz = 36U;
    I2C1_CR2   = freq_mhz;

    /* Standard-mode: CCR ~= PCLK1/(2*I2C_SPEED) */
    unsigned int ccr = (PCLK1_HZ + (2U*I2C_SPEED_HZ-1U)) / (2U*I2C_SPEED_HZ);
    if (ccr < 4U)     ccr = 4U;
    if (ccr > 0xFFFU) ccr = 0xFFFU;
    I2C1_CCR   = ccr;

    /* TRISE(SM) = FREQ(MHz) + 1 */
    I2C1_TRISE = freq_mhz + 1U;
}

void I2C1_Init(void)
{
    /* Clocks */
    SET_BITS(RCC_APB2ENR, RCC_APB2ENR_AFIOEN);
    SET_BITS(RCC_APB2ENR, RCC_APB2ENR_IOPBEN);
    SET_BITS(RCC_APB1ENR, RCC_APB1ENR_I2C1EN);

    /* PB6/PB7 = AF Open-Drain 50MHz (0xB per nibble) */
    GPIOB_CRL &= ~((GPIO_CRL_NIBBLE_MASK << (6U*4U)) |
                   (GPIO_CRL_NIBBLE_MASK << (7U*4U)));
    GPIOB_CRL |=  ((unsigned int)GPIO_MODE_AF_OD_50MHz << (6U*4U)) |
                  ((unsigned int)GPIO_MODE_AF_OD_50MHz << (7U*4U));

    /* Reset */
    SET_BITS(I2C1_CR1, I2C_CR1_SWRST);
    CLR_BITS(I2C1_CR1, I2C_CR1_SWRST);

    /* Timing */
    i2c1_timing_stdmode();

    /* Enable */
    SET_BITS(I2C1_CR1, I2C_CR1_PE);
    tiny_delay(500U);
}

void I2C1_Start(void)
{
    volatile unsigned int t = I2C_TIMEOUT;
    SET_BITS(I2C1_CR1, I2C_CR1_START);
    while (!(I2C1_SR1 & I2C_SR1_SB)) { if(--t==0U) break; }
    (void)I2C1_SR1; /* clear SB via SR1 read before DR write */
}

int I2C1_Address(unsigned char addr7, int dir)
{
    volatile unsigned int t = I2C_TIMEOUT;
    I2C1_DR = ((unsigned int)(addr7 << 1)) | ((dir & 1) ? 1U : 0U);
    while (!(I2C1_SR1 & I2C_SR1_ADDR)) {
        if (I2C1_SR1 & I2C_SR1_AF) { CLR_BITS(I2C1_SR1, I2C_SR1_AF); return -1; }
        if (--t == 0U) return -2; /* timeout */
    }
    (void)I2C1_SR1; (void)I2C1_SR2; /* clear ADDR */
    return 0;
}

int I2C1_WriteByte(unsigned char data)
{
    volatile unsigned int t;

    /* wait TXE */
    t = I2C_TIMEOUT;
    while (!(I2C1_SR1 & I2C_SR1_TXE)) { if(--t==0U) return -1; }

    I2C1_DR = data;

    /* wait BTF (byte transfer finished) */
    t = I2C_TIMEOUT;
    while (!(I2C1_SR1 & I2C_SR1_BTF)) {
        if (I2C1_SR1 & I2C_SR1_AF) { CLR_BITS(I2C1_SR1, I2C_SR1_AF); return -2; }
        if (--t == 0U) return -3;
    }
    return 0;
}

unsigned char I2C1_ReadAck(void)
{
    volatile unsigned int t = I2C_TIMEOUT;
    SET_BITS(I2C1_CR1, I2C_CR1_ACK);
    while (!(I2C1_SR1 & I2C_SR1_RXNE)) { if(--t==0U) break; }
    return (unsigned char)I2C1_DR;
}

unsigned char I2C1_ReadNack(void)
{
    volatile unsigned int t = I2C_TIMEOUT;
    CLR_BITS(I2C1_CR1, I2C_CR1_ACK);
    SET_BITS(I2C1_CR1, I2C_CR1_STOP);
    while (!(I2C1_SR1 & I2C_SR1_RXNE)) { if(--t==0U) break; }
    return (unsigned char)I2C1_DR;
}

void I2C1_Stop(void)
{
    SET_BITS(I2C1_CR1, I2C_CR1_STOP);
    tiny_delay(100U);
}

int I2C1_Probe(unsigned char addr7)
{
    I2C1_Start();
    if (I2C1_Address(addr7, 0) == 0) { I2C1_Stop(); return 1; }
    I2C1_Stop();
    return 0;
}

int I2C1_ReadReg1(unsigned char addr7, unsigned char reg, unsigned char *val)
{
    if (!val) return -1;
    I2C1_Start();
    if (I2C1_Address(addr7, 0) != 0) { I2C1_Stop(); return -2; }
    if (I2C1_WriteByte(reg) != 0)    { I2C1_Stop(); return -3; }
    I2C1_Start();
    if (I2C1_Address(addr7, 1) != 0) { I2C1_Stop(); return -4; }
    *val = I2C1_ReadNack();          /* STOP inside ReadNack */
    return 0;
}
