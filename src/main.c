#include "i2c_config.h"
#include "i2c_interface.h"

#define WHO_AM_I_REG  0x75U

static void delay(volatile unsigned int t){ while(t--){ __asm__ volatile("nop"); } }

int main(void)
{
    unsigned char who = 0U;
    int ok = 0;

    /* LEDs init: PC13 ERR (active-LOW), PC14 OK (active-HIGH) */
    LEDS_INIT_PC13_PC14();
    LED_ERR_ON();   /* show life */
    LED_OK_OFF();

    /* I2C init */
    I2C1_Init();

    /* Try both addresses: 0x68 (AD0=GND), then 0x69 (AD0=VCC) */
    if (I2C1_Probe(0x68)) {
        if (I2C1_ReadReg1(0x68, WHO_AM_I_REG, &who) == 0 && who == 0x68) ok = 1;
    } else if (I2C1_Probe(0x69)) {
        if (I2C1_ReadReg1(0x69, WHO_AM_I_REG, &who) == 0 && who == 0x68) ok = 1;
    }

    if (ok) {
        LED_ERR_OFF();
        LED_OK_ON();
        while (1) { delay(300000U); }   /* solid OK */
    }

    /* Error: blink ERR forever */
    while (1) {
        LED_ERR_ON();  delay(200000U);
        LED_ERR_OFF(); delay(200000U);
    }
}
