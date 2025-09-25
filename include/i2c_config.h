#ifndef I2C_CONFIG_H
#define I2C_CONFIG_H

/* ================= Helper Macros ================= */
#define BIT(n)                (1U << (n))
#define REG32(addr)           (*(volatile unsigned int *)(addr))
#define SET_BITS(reg, mask)   ((reg) |= (mask))
#define CLR_BITS(reg, mask)   ((reg) &= ~(mask))

/* Nibble mask for GPIO CRL/CRH (per-pin 4-bit field) */
#define GPIO_CRL_NIBBLE_MASK  0xFU

/* ================= Bus/Clock Settings =================
   Typical Blue Pill: SYSCLK=72MHz, APB1=36MHz → I2C timings assume PCLK1=36MHz.
   If you are running HSI 8MHz (no PLL), set PCLK1_HZ to 8000000U.
*/
#define PCLK1_HZ       36000000U   /* APB1 clock in Hz (edit to actual) */
#define I2C_SPEED_HZ   100000U     /* 100kHz Standard Mode by default   */
#define I2C_TIMEOUT    1000000U    /* simple polling timeout counter    */

/* ================= Base Addresses ================= */
#define RCC_BASE       0x40021000U
#define AFIO_BASE      0x40010000U
#define GPIOA_BASE     0x40010800U
#define GPIOB_BASE     0x40010C00U
#define GPIOC_BASE     0x40011000U
#define I2C1_BASE      0x40005400U

/* ================= RCC Registers ================= */
#define RCC_APB2ENR    REG32(RCC_BASE + 0x18U)
#define RCC_APB1ENR    REG32(RCC_BASE + 0x1CU)

/* RCC APB2ENR bits */
#define RCC_APB2ENR_AFIOEN   BIT(0)
#define RCC_APB2ENR_IOPAEN   BIT(2)
#define RCC_APB2ENR_IOPBEN   BIT(3)
#define RCC_APB2ENR_IOPCEN   BIT(4)

/* RCC APB1ENR bits */
#define RCC_APB1ENR_I2C1EN   BIT(21)

/* ================= AFIO (Remap) ================= */
#define AFIO_MAPR      REG32(AFIO_BASE + 0x04U)
/* I2C1_REMAP = 0 -> PB6/PB7; 1 -> PB8/PB9 */
#define AFIO_MAPR_I2C1_REMAP BIT(1)

/* ================= GPIO Registers ================= */
#define GPIOA_CRL      REG32(GPIOA_BASE + 0x00U)
#define GPIOA_CRH      REG32(GPIOA_BASE + 0x04U)
#define GPIOA_IDR      REG32(GPIOA_BASE + 0x08U)
#define GPIOA_ODR      REG32(GPIOA_BASE + 0x0CU)

#define GPIOB_CRL      REG32(GPIOB_BASE + 0x00U)
#define GPIOB_CRH      REG32(GPIOB_BASE + 0x04U)
#define GPIOB_IDR      REG32(GPIOB_BASE + 0x08U)
#define GPIOB_ODR      REG32(GPIOB_BASE + 0x0CU)

#define GPIOC_CRL      REG32(GPIOC_BASE + 0x00U)
#define GPIOC_CRH      REG32(GPIOC_BASE + 0x04U)
#define GPIOC_IDR      REG32(GPIOC_BASE + 0x08U)
#define GPIOC_ODR      REG32(GPIOC_BASE + 0x0CU)

/* CRL/CRH nibble format per pin: [CNF1 CNF0 MODE1 MODE0]
   For I2C SCL/SDA -> Alternate Function Open-Drain, 50 MHz:
   CNF=10, MODE=11 => 0b1011 = 0xB */
#define GPIO_MODE_AF_OD_50MHz  (0xBU)

/* ================= I2C1 Registers ================= */
#define I2C1_CR1       REG32(I2C1_BASE + 0x00U)
#define I2C1_CR2       REG32(I2C1_BASE + 0x04U)
#define I2C1_OAR1      REG32(I2C1_BASE + 0x08U)
#define I2C1_OAR2      REG32(I2C1_BASE + 0x0CU)
#define I2C1_DR        REG32(I2C1_BASE + 0x10U)
#define I2C1_SR1       REG32(I2C1_BASE + 0x14U)
#define I2C1_SR2       REG32(I2C1_BASE + 0x18U)
#define I2C1_CCR       REG32(I2C1_BASE + 0x1CU)
#define I2C1_TRISE     REG32(I2C1_BASE + 0x20U)

/* I2C: CR1 bits */
#define I2C_CR1_PE      BIT(0)
#define I2C_CR1_START   BIT(8)
#define I2C_CR1_STOP    BIT(9)
#define I2C_CR1_ACK     BIT(10)
#define I2C_CR1_POS     BIT(11)
#define I2C_CR1_SWRST   BIT(15)

/* I2C: CR2 fields */
#define I2C_CR2_FREQ_Pos  0U
#define I2C_CR2_FREQ_Msk  (0x3FU << I2C_CR2_FREQ_Pos)

/* I2C: CCR bits */
#define I2C_CCR_FS     BIT(15)
#define I2C_CCR_DUTY   BIT(14)

/* I2C: SR1 flags */
#define I2C_SR1_SB     BIT(0)
#define I2C_SR1_ADDR   BIT(1)
#define I2C_SR1_BTF    BIT(2)
#define I2C_SR1_RXNE   BIT(6)
#define I2C_SR1_TXE    BIT(7)
#define I2C_SR1_ARLO   BIT(9)
#define I2C_SR1_AF     BIT(10)

/* I2C: SR2 flags */
#define I2C_SR2_BUSY   BIT(1)

/* ================= Pins ================= */
/* I2C1 default (no remap): PB6=SCL, PB7=SDA */
#define I2C_SCL_PIN    6U
#define I2C_SDA_PIN    7U

/* LEDs moved to Port C: PC0 = OK, PC2 = ERR */
#define LED_OK_ON()    (GPIOC_ODR |= BIT(0))
#define LED_OK_OFF()   (GPIOC_ODR &= ~BIT(0))
#define LED_ERR_ON()   (GPIOC_ODR |= BIT(2))
#define LED_ERR_OFF()  (GPIOC_ODR &= ~BIT(2))

#endif /* I2C_CONFIG_H */

