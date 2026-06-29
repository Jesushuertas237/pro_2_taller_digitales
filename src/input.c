#include "input.h"
#include "xil_io.h"

/* Ciclos consecutivos con el mismo valor necesarios para aceptar un cambio */
#define DEBOUNCE_COUNT 2

/* Estado del debounce para botones */
static unsigned int btn_previous     = 0;
static int          btn_stable_count = 0;
static unsigned int btn_candidate    = 0;

/* Estado del debounce para switches */
static unsigned int sw_previous     = 0;
static int          sw_stable_count = 0;
static unsigned int sw_candidate    = 0;

/* Configura los puertos GPIO de botones y switches como entradas (TRI_REG = 1). */
void input_init(void) {
    Xil_Out32(GPIO_BUTTONS_BASE + GPIO_TRI_REG, 0xFFFFFFFF);
    Xil_Out32(GPIO_SWITCHES_BASE + GPIO_TRI_REG, 0xFFFFFFFF);
}

/*
 * Lee los botones con maquina de debounce.
 * Retorna mascara de bits con los flancos de subida (0->1) detectados,
 * o 0 si no hay cambio estable aun.
 */
unsigned int input_read_buttons(void) {
    unsigned int raw = Xil_In32(GPIO_BUTTONS_BASE + GPIO_DATA_REG) & 0xF;

    if (raw != btn_candidate) {
        /* Valor cambio: reiniciar contador y esperar estabilidad */
        btn_candidate    = raw;
        btn_stable_count = 1;
        return 0;
    }

    btn_stable_count++;
    if (btn_stable_count >= DEBOUNCE_COUNT) {
        /* Valor estable: detectar flanco de subida respecto al estado anterior */
        unsigned int pressed = raw & ~btn_previous;
        btn_previous     = raw;
        btn_stable_count = 0;
        return pressed;
    }

    return 0;
}

/*
 * Lee los switches con maquina de debounce.
 * Retorna mascara de bits con los flancos de subida (0->1) detectados,
 * o 0 si no hay cambio estable aun.
 */
unsigned int input_read_switches(void) {
    unsigned int raw = Xil_In32(GPIO_SWITCHES_BASE + GPIO_DATA_REG) & 0xFFFF;

    if (raw != sw_candidate) {
        /* Valor cambio: reiniciar contador y esperar estabilidad */
        sw_candidate    = raw;
        sw_stable_count = 1;
        return 0;
    }

    sw_stable_count++;
    if (sw_stable_count >= DEBOUNCE_COUNT) {
        /* Valor estable: detectar flanco de subida respecto al estado anterior */
        unsigned int pressed = raw & ~sw_previous;
        sw_previous     = raw;
        sw_stable_count = 0;
        return pressed;
    }

    return 0;
}
