#include "game_state.h"
#include "input.h"
#include "vga.h"

static Buttons prev_buttons = {0, 0, 0, 0, 0};
static unsigned int btnc_last_press = 0;
static unsigned int config_last_press = 0;
static unsigned int press_counter = 0;

#define BTNC_DEBOUNCE_ITER 1
#define CONFIG_DEBOUNCE_ITER 5

void game_init(GameContext *ctx) {
    ctx->current_state = STATE_INIT;
    ctx->game_mode = MODE_SINGLE;
    ctx->fpga_role = ROLE_NONE;
    ctx->is_ready_to_start = 0;
    delay_ms(100);
}

unsigned char is_btnc_pressed(Buttons buttons) {
    if (buttons.btnc && !prev_buttons.btnc) {
        if (press_counter - btnc_last_press > BTNC_DEBOUNCE_ITER) {
            btnc_last_press = press_counter;
            return 1;
        }
    }
    return 0;
}

unsigned char is_config_button_pressed(Buttons buttons) {
    unsigned char config_pressed = (buttons.btnl && !prev_buttons.btnl) ||
                                   (buttons.btnr && !prev_buttons.btnr) ||
                                   (buttons.btnu && !prev_buttons.btnu);

    if (config_pressed && (press_counter - config_last_press > CONFIG_DEBOUNCE_ITER)) {
        config_last_press = press_counter;
        return 1;
    }
    return 0;
}

void game_handle_input(GameContext *ctx) {
    Buttons buttons = read_buttons();

    switch (ctx->current_state) {
        case STATE_INIT:
            if (is_config_button_pressed(buttons)) {
                if (buttons.btnl && !prev_buttons.btnl) {
                    ctx->game_mode = MODE_SINGLE;
                    ctx->current_state = STATE_SINGLE_PLAYER;
                } else if (buttons.btnr && !prev_buttons.btnr) {
                    ctx->game_mode = MODE_MULTIPLAYER;
                    ctx->current_state = STATE_MULTIPLAYER;
                }
            }
            break;

        case STATE_SINGLE_PLAYER:
            if (is_btnc_pressed(buttons)) {
                ctx->fpga_role = ROLE_MASTER;
                ctx->current_state = STATE_GAME;
            }
            break;

        case STATE_MULTIPLAYER:
            if (is_config_button_pressed(buttons)) {
                if (buttons.btnu && !prev_buttons.btnu) {
                    ctx->fpga_role = ROLE_MASTER;
                    ctx->current_state = STATE_WAIT_START;
                } else if (buttons.btnl && !prev_buttons.btnl) {
                    ctx->fpga_role = ROLE_SLAVE;
                    ctx->current_state = STATE_WAIT_START;
                }
            }
            break;

        case STATE_WAIT_START:
            if (is_btnc_pressed(buttons)) {
                ctx->current_state = STATE_GAME;
            }
            break;

        case STATE_GAME:
            if (is_btnc_pressed(buttons)) {
                ctx->current_state = STATE_INIT;
                ctx->game_mode = MODE_SINGLE;
                ctx->fpga_role = ROLE_NONE;
            }
            break;
    }

    prev_buttons = buttons;
    press_counter++;
}

void game_update(GameContext *ctx) {
    game_handle_input(ctx);
}
