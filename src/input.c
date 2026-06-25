#include "input.h"

static void delay_us(unsigned int us) {
    for (volatile unsigned int i = 0; i < us * 10; i++);
}

static unsigned int read_gpio(unsigned int base, unsigned int offset) {
    volatile unsigned int *gpio = (volatile unsigned int *)(base + offset);
    return *gpio;
}

Switches read_switches(void) {
    Switches sw;
    unsigned int val1, val2;

    val1 = read_gpio(GPIO_SWITCHES_BASE, 0x8);
    delay_us(50);
    val2 = read_gpio(GPIO_SWITCHES_BASE, 0x8);

    sw.value = (val1 == val2) ? val1 : val2;

    return sw;
}

Buttons read_buttons(void) {
    Buttons btn;
    unsigned int raw1, raw2;

    raw1 = read_gpio(GPIO_BUTTONS_BASE, 0x0);
    delay_us(100);
    raw2 = read_gpio(GPIO_BUTTONS_BASE, 0x0);

    unsigned int stable = (raw1 == raw2) ? raw1 : raw2;

    btn.btnc = (stable >> 0) & 1;
    btn.btnu = (stable >> 1) & 1;
    btn.btnl = (stable >> 2) & 1;
    btn.btnr = (stable >> 3) & 1;
    btn.btnd = (stable >> 4) & 1;

    return btn;
}
