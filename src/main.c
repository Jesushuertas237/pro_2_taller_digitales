#include "vga.h"
#include "font.h"
#include "input.h"

/* Estados de la máquina */
typedef enum {
    STATE_INICIO,
    STATE_ROLE_SELECT,
    STATE_MAESTRO,
    STATE_ESCLAVO,
    STATE_PLAY
} STATE;

STATE current_state = STATE_INICIO;
STATE next_state = STATE_INICIO;

/* Dibujar pantalla de inicio */
void screen_inicio() {
    clear_screen(COLOR_BLACK);
    draw_string(50, 50, "INICIO", COLOR_WHITE);
    draw_string(50, 100, "SW0=Play", COLOR_CYAN_LIGHT);
    draw_string(50, 150, "SW1=Role", COLOR_CYAN_LIGHT);
}

/* Dibujar pantalla de seleccionar rol */
void screen_role_select() {
    clear_screen(COLOR_BLACK);
    draw_string(50, 50, "ROLE_SELECT", COLOR_WHITE);
    draw_string(50, 100, "SW15=Master", COLOR_GREEN_LIGHT);
    draw_string(50, 150, "SW14=Slave", COLOR_GREEN_LIGHT);
}

/* Dibujar pantalla maestro */
void screen_maestro() {
    clear_screen(COLOR_BLACK);
    draw_string(50, 50, "MAESTRO", COLOR_WHITE);
    draw_string(50, 100, "BTNC=Play", COLOR_MAGENTA_LIGHT);
    draw_string(50, 150, "BTNC=Inicio", COLOR_MAGENTA_LIGHT);
}

/* Dibujar pantalla esclavo */
void screen_esclavo() {
    clear_screen(COLOR_BLACK);
    draw_string(50, 50, "ESCLAVO", COLOR_WHITE);
    draw_string(50, 100, "BTNC=Play", COLOR_MAGENTA_LIGHT);
    draw_string(50, 150, "BTNC=Inicio", COLOR_MAGENTA_LIGHT);
}

/* Dibujar pantalla play */
void screen_play() {
    clear_screen(COLOR_BLACK);
    draw_string(50, 50, "PLAY", COLOR_WHITE);
    draw_string(50, 100, "BTNC=Inicio", COLOR_YELLOW);
}

/* Procesar transiciones de estado */
void process_state() {
    unsigned int btn_pressed = input_read_buttons();
    unsigned int sw_pressed = input_read_switches();

    switch (current_state) {
        case STATE_INICIO:
            if (sw_pressed & (1 << 0)) {  /* SW0 presionado */
                next_state = STATE_PLAY;
            } else if (sw_pressed & (1 << 1)) {  /* SW1 presionado */
                next_state = STATE_ROLE_SELECT;
            }
            break;

        case STATE_ROLE_SELECT:
            if (sw_pressed & (1 << 15)) {  /* SW15 presionado */
                next_state = STATE_MAESTRO;
            } else if (sw_pressed & (1 << 14)) {  /* SW14 presionado */
                next_state = STATE_ESCLAVO;
            }
            break;

        case STATE_MAESTRO:
            if (btn_pressed & (1 << 0)) {  /* BTNC presionado */
                next_state = STATE_PLAY;
            }
            break;

        case STATE_ESCLAVO:
            if (btn_pressed & (1 << 0)) {  /* BTNC presionado */
                next_state = STATE_PLAY;
            }
            break;

        case STATE_PLAY:
            if (btn_pressed & (1 << 0)) {  /* BTNC presionado */
                next_state = STATE_INICIO;
            }
            break;
    }
}

int main(void) {
    /* Inicializar módulos */
    input_init();

    current_state = STATE_INICIO;
    STATE last_drawn_state = -1;  /* Controlar redibujo */

    /* Dibujar pantalla inicial */
    switch (current_state) {
        case STATE_INICIO:
            screen_inicio();
            break;
        case STATE_ROLE_SELECT:
            screen_role_select();
            break;
        case STATE_MAESTRO:
            screen_maestro();
            break;
        case STATE_ESCLAVO:
            screen_esclavo();
            break;
        case STATE_PLAY:
            screen_play();
            break;
    }
    last_drawn_state = current_state;

    while (1) {
        /* Procesar input */
        process_state();

        /* Transicionar si hay cambio */
        if (next_state != current_state) {
            current_state = next_state;
        }

        /* Redibujar SOLO si cambió de estado */
        if (current_state != last_drawn_state) {
            switch (current_state) {
                case STATE_INICIO:
                    screen_inicio();
                    break;
                case STATE_ROLE_SELECT:
                    screen_role_select();
                    break;
                case STATE_MAESTRO:
                    screen_maestro();
                    break;
                case STATE_ESCLAVO:
                    screen_esclavo();
                    break;
                case STATE_PLAY:
                    screen_play();
                    break;
            }
            last_drawn_state = current_state;
        }

        delay_ms(5);  /* Polling: 5ms */
    }

    return 0;
}
