#include "ui.h"
#include "vga.h"
#include "font.h"

void draw_state_display(GameContext *ctx) {
    clear_screen(COLOR_BLACK);

    /* Mostrar estado actual en letras pequeñas */
    const char *state_str = "";

    switch (ctx->current_state) {
        case STATE_INIT:
            state_str = "ESTADO: INIT";
            draw_string(10, 10, state_str, COLOR_WHITE);
            draw_string(10, 30, "BTNL=SINGLE  BTNR=MULTI", COLOR_CYAN_LIGHT);
            break;

        case STATE_SINGLE_PLAYER:
            state_str = "ESTADO: SINGLE PLAYER";
            draw_string(10, 10, state_str, COLOR_WHITE);
            draw_string(10, 30, "BTNC=START", COLOR_GREEN_LIGHT);
            break;

        case STATE_MULTIPLAYER:
            state_str = "ESTADO: MULTIPLAYER";
            draw_string(10, 10, state_str, COLOR_WHITE);
            draw_string(10, 30, "BTNU=MASTER  BTNL=SLAVE", COLOR_GREEN_LIGHT);
            break;

        case STATE_WAIT_START:
            state_str = "ESTADO: WAIT START";
            draw_string(10, 10, state_str, COLOR_WHITE);
            draw_string(10, 30, "BTNC=START GAME", COLOR_YELLOW);
            if (ctx->fpga_role == ROLE_MASTER) {
                draw_string(10, 50, "ROL: MASTER", COLOR_RED_LIGHT);
            } else if (ctx->fpga_role == ROLE_SLAVE) {
                draw_string(10, 50, "ROL: SLAVE", COLOR_RED_LIGHT);
            }
            break;

        case STATE_GAME:
            state_str = "ESTADO: GAME";
            draw_string(10, 10, state_str, COLOR_WHITE);
            draw_string(10, 30, "BTNC=MENU", COLOR_YELLOW);
            draw_border(COLOR_WHITE);
            break;
    }
}
