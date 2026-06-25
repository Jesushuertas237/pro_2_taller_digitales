#ifndef GAME_STATE_H
#define GAME_STATE_H

typedef enum {
    STATE_INIT,
    STATE_SINGLE_PLAYER,
    STATE_MULTIPLAYER,
    STATE_WAIT_START,
    STATE_GAME
} GameState;

typedef enum {
    MODE_SINGLE,
    MODE_MULTIPLAYER
} GameMode;

typedef enum {
    ROLE_NONE,
    ROLE_MASTER,
    ROLE_SLAVE
} FPGARole;

typedef struct {
    GameState current_state;
    GameMode game_mode;
    FPGARole fpga_role;
    unsigned char is_ready_to_start;
} GameContext;

void game_init(GameContext *ctx);
void game_update(GameContext *ctx);
void game_handle_input(GameContext *ctx);

#endif
