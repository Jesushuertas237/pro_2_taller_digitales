#include "game_state.h"
#include "vga.h"
#include "ui.h"

int main(void) {
    clear_screen(COLOR_BLACK);
    delay_ms(100);

    GameContext game_ctx;
    game_init(&game_ctx);
    delay_ms(100);

    unsigned int last_state = 0xFF;
    while (1) {
        game_update(&game_ctx);

        if (game_ctx.current_state != last_state) {
            last_state = game_ctx.current_state;
            draw_state_display(&game_ctx);
        }

        delay_ms(10);
    }

    return 0;
}
