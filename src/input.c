#include "input.h"
#include "xil_io.h"

#define DEBOUNCE_COUNT 2

/* Debounce state for buttons */
static unsigned int btn_previous = 0;
static int btn_stable_count = 0;
static unsigned int btn_candidate = 0;

/* Debounce state for switches */
static unsigned int sw_previous = 0;
static int sw_stable_count = 0;
static unsigned int sw_candidate = 0;

void input_init(void) {
    /* Configure GPIO ports as inputs (TRI_REG = 1) */
    Xil_Out32(GPIO_BUTTONS_BASE + GPIO_TRI_REG, 0xFFFFFFFF);
    Xil_Out32(GPIO_SWITCHES_BASE + GPIO_TRI_REG, 0xFFFFFFFF);
}

/* Read buttons with debounce state machine. Returns rising edges (0->1 transitions). */
unsigned int input_read_buttons(void) {
    unsigned int raw = Xil_In32(GPIO_BUTTONS_BASE + GPIO_DATA_REG) & 0xF;

    /* Raw value changed, reset counter */
    if (raw != btn_candidate) {
        btn_candidate = raw;
        btn_stable_count = 1;
        return 0;
    }

    /* Same value, increment stability counter */
    btn_stable_count++;
    if (btn_stable_count >= DEBOUNCE_COUNT) {
        /* State stable, detect rising edge */
        unsigned int pressed = raw & ~btn_previous;
        btn_previous = raw;
        btn_stable_count = 0;
        return pressed;
    }

    return 0;
}

/* Read switches with debounce state machine. Returns rising edges (0->1 transitions). */
unsigned int input_read_switches(void) {
    unsigned int raw = Xil_In32(GPIO_SWITCHES_BASE + GPIO_DATA_REG) & 0xFFFF;

    /* Raw value changed, reset counter */
    if (raw != sw_candidate) {
        sw_candidate = raw;
        sw_stable_count = 1;
        return 0;
    }

    /* Same value, increment stability counter */
    sw_stable_count++;
    if (sw_stable_count >= DEBOUNCE_COUNT) {
        /* State stable, detect rising edge */
        unsigned int pressed = raw & ~sw_previous;
        sw_previous = raw;
        sw_stable_count = 0;
        return pressed;
    }

    return 0;
}
