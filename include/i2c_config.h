#ifndef I2C_CONFIG_H
#define I2C_CONFIG_H

/* ===== Helpers ===== */
#define BIT(n)                (1U << (n))
#define REG32(addr)           (*(volatile unsigned int *)(addr))
#define SET_BITS(reg, mask)   ((reg) |= (mask))
#define CLR_BITS(reg, mask)   ((reg) &= ~(mask))

/* ===== Clocks (default safe) =====*/

#define PCLK1_HZ       8000000U
#define I2C_SPEED_HZ   100000U
#define I2C_TIMEOUT    1000000U

/* ===== Base addresses ===== */
#define RCC_BASE       0x40021000U
#define AFIO_BASE      0x40010000U
#define GPIOA_BASE     0x40010800U
#define GPIOB_BASE     0x40010C00U
#define GPIOC_BASE     0x40011000U
#define I2C1_BASE      0x40005400U

/* ===== RCC ===== */
#define RCC_APB2ENR    REG32(RCC_BASE + 0x18U)
#define RCC_APB1ENR    REG32(RCC_BASE + 0x1CU)
#define RCC_APB2ENR_AFIOEN   BIT(0)
#define RCC_APB2ENR_IOPAEN   BIT(2)
#define RCC_APB2ENR_IOPBEN   BIT(3)
#define RCC_APB2ENR_IOPCEN   BIT(4)
#define RCC_APB1ENR_I2C1EN   BIT(21)

/* ===== GPIO ===== */
#define GPIOA_CRL      REG32(GPIOA_BASE + 0x00U)
#define GPIOA_CRH      REG32(GPIOA_BASE + 0x04U)
#define GPIOB_CRL      REG32(GPIOB_BASE + 0x00U)
#define GPIOB_CRH      REG32(GPIOB_BASE + 0x04U)
#define GPIOB_ODR      REG32(GPIOB_BASE + 0x0CU)
#define GPIOC_CRL      REG32(GPIOC_BASE + 0x00U)
#define GPIOC_CRH      REG32(GPIOC_BASE + 0x04U)
#define GPIOC_BSRR     REG32(GPIOC_BASE + 0x10U)

#define GPIO_CRL_NIBBLE_MASK  0xFU
#define GPIO_MODE_AF_OD_50MHz 0xBU

/* ===== I2C1 ===== */
#define I2C1_CR1       REG32(I2C1_BASE + 0x00U)
#define I2C1_CR2       REG32(I2C1_BASE + 0x04U)
#define I2C1_OAR1      REG32(I2C1_BASE + 0x08U)
#define I2C1_OAR2      REG32(I2C1_BASE + 0x0CU)
#define I2C1_DR        REG32(I2C1_BASE + 0x10U)
#define I2C1_SR1       REG32(I2C1_BASE + 0x14U)
#define I2C1_SR2       REG32(I2C1_BASE + 0x18U)
#define I2C1_CCR       REG32(I2C1_BASE + 0x1CU)
#define I2C1_TRISE     REG32(I2C1_BASE + 0x20U)

/* I2C bits */
#define I2C_CR1_PE      BIT(0)
#define I2C_CR1_START   BIT(8)
#define I2C_CR1_STOP    BIT(9)
#define I2C_CR1_ACK     BIT(10)
#define I2C_CR1_SWRST   BIT(15)
#define I2C_SR1_SB      BIT(0)
#define I2C_SR1_ADDR    BIT(1)
#define I2C_SR1_BTF     BIT(2)
#define I2C_SR1_RXNE    BIT(6)
#define I2C_SR1_TXE     BIT(7)
#define I2C_SR1_AF      BIT(10)
#define I2C_SR2_BUSY    BIT(1)

/* ===== I2C1 default pins  ===== */
#define I2C_SCL_PIN    6U   /* PB6 */
#define I2C_SDA_PIN    7U   /* PB7 */

/* ===== LEDs: PC13 ERR (active-LOW), PC14 OK (active-HIGH) ===== */
#define LEDS_INIT_PC13_PC14()                                      \
    do {                                                           \
        SET_BITS(RCC_APB2ENR, RCC_APB2ENR_IOPCEN);                 \
        /* PC13, PC14 are in CRH: nibbles @ 20,24 */               \
        GPIOC_CRH &= ~((GPIO_CRL_NIBBLE_MASK << 20U) |             \
                       (GPIO_CRL_NIBBLE_MASK << 24U));             \
        /* Output push-pull, 2MHz = 0x2 */                         \
        GPIOC_CRH |=  ((0x2U << 20U) | (0x2U << 24U));             \
        /* OFF: PC13 HIGH, PC14 LOW */                             \
        GPIOC_BSRR = BIT(13);        /* set 13 -> HIGH (ERR off)*/ \
        GPIOC_BSRR = BIT(14+16);     /* reset 14 -> LOW (OK off) */\
    } while(0)

#define LED_ERR_ON()    (GPIOC_BSRR = BIT(13+16))  /* PC13 LOW  */
#define LED_ERR_OFF()   (GPIOC_BSRR = BIT(13))     /* PC13 HIGH */
#define LED_OK_ON()     (GPIOC_BSRR = BIT(14))     /* PC14 HIGH */
#define LED_OK_OFF()    (GPIOC_BSRR = BIT(14+16))  /* PC14 LOW  */

#endif /* I2C_CONFIG_H */
