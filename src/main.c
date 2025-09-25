/* main.c - IMU (MPU6050) bring-up with LEDs on PC0 (OK) and PC2 (ERR) */
#include "i2c_config.h"
#include "i2c_interface.h"

/* ---- IMU (MPU6050) addresses/regs ---- */
#define MPU_ADDR_LOW    0x68U   /* AD0=GND */
#define MPU_ADDR_HIGH   0x69U   /* AD0=VCC */
#define MPU_WHOAMI_REG  0x75U   /* should read 0x68 */

/* Small busy-wait delay */
static void small_delay(volatile unsigned int ticks)
{
    while (ticks--) { __asm__ volatile ("nop"); }
}

/* Configure PC0 & PC2 as output push-pull (2MHz) */
static void LED_GPIO_Init(void)
{
    /* Enable GPIOC clock */
    SET_BITS(RCC_APB2ENR, RCC_APB2ENR_IOPCEN);

    /* PC0 nibble = bits [3:0], PC2 nibble = bits [11:8] in GPIOC_CRL */
    /* Clear both nibbles */
    GPIOC_CRL &= ~((GPIO_CRL_NIBBLE_MASK << (0U * 4U)) |
                   (GPIO_CRL_NIBBLE_MASK << (2U * 4U)));
    /* MODE=0b10 (2MHz), CNF=0b00 (General purpose push-pull) → 0x2 */
    GPIOC_CRL |=  ((0x2U << (0U * 4U)) |
                   (0x2U << (2U * 4U)));

    /* Initial OFF */
    LED_OK_OFF();
    LED_ERR_OFF();
}

/* Simple bus scan helper: return 1 if any device at 'addr7' ACKs write */
static int I2C_Probe(unsigned int addr7)
{
    int ok = 0;
    if (I2C1_Start() == 0) {
        if (I2C1_SendAddress(addr7, 0) == 0) ok = 1;  /* write=0 */
        I2C1_Stop();
    }
    return ok;
}

/* Read WHO_AM_I (1 byte) from given 7-bit addr; return 0 on OK */
static int IMU_Read_WHOAMI(unsigned int addr7, unsigned int *whoami)
{
    if (!whoami) return -1;

    /* Write phase: send register address */
    if (I2C1_Start() != 0) return -1;
    if (I2C1_SendAddress(addr7, 0) != 0) { I2C1_Stop(); return -1; }
    if (I2C1_WriteByte(MPU_WHOAMI_REG) != 0) { I2C1_Stop(); return -1; }

    /* Repeated START + read 1 byte (NACK + STOP) */
    if (I2C1_Start() != 0) { I2C1_Stop(); return -1; }
    if (I2C1_SendAddress(addr7, 1) != 0) { I2C1_Stop(); return -1; }
    if (I2C1_ReadByte_NACK(whoami) != 0) { I2C1_Stop(); return -1; }

    /* STOP is issued in ReadByte_NACK */
    return 0;
}

int main(void)
{
    unsigned int whoami = 0U;
    int status = -1;
    int present = 0;

    /* 1) Init I2C1 (enables clocks, config PB6/PB7 AF-OD, timing, enable) */
    I2C1_Init();

    /* 2) Init LEDs on PC0 (OK) and PC2 (ERR) */
    LED_GPIO_Init();

    /* 3) Probe IMU address (prefer 0x68 if AD0=GND, else 0x69) */
    if (I2C_Probe(MPU_ADDR_LOW)) {
        status = IMU_Read_WHOAMI(MPU_ADDR_LOW, &whoami);
        present = 1;
    } else if (I2C_Probe(MPU_ADDR_HIGH)) {
        status = IMU_Read_WHOAMI(MPU_ADDR_HIGH, &whoami);
        present = 1;
    } else {
        present = 0;
    }

    /* 4) LEDs indication */
    if (present && status == 0 && whoami == 0x68U) {
        /* IMU present and WHO_AM_I correct => success */
        LED_ERR_OFF();
        LED_OK_ON();
    } else {
        /* error condition */
        LED_OK_OFF();
        LED_ERR_ON();
    }

    /* 5) Idle loop: blink ERR slowly if error */
    while (1) {
        if (!(present && status == 0 && whoami == 0x68U)) {
            /* toggle PC2 */
            GPIOC_ODR ^= BIT(2);
        }
        small_delay(300000U);
    }
}

