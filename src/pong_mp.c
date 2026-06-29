#include "vga.h"
#include "input.h"
#include "ui_text.h"
#include "spi.h"

/* Dimensiones y fisica del juego (deben coincidir con pong_paddles.c) */
#define PAD_W      8
#define PAD_H      60
#define PAD_SPEED  10
#define PLAYER_X   24
#define CPU_X      608
#define BALL_SZ    8
#define BALL_VX    8
#define BALL_VY    5
#define SCORE_Y    8
#define TOP_WALL   18
#define WIN_SCORE  3

/* Ciclos de espera por frame del maestro; el esclavo sigue su cadencia via SPI */
#define MP_TICK 7500U

#define BTN_C (1u << 0)
#define BTN_U (1u << 1)
#define BTN_D (1u << 4)

/* Flags en el byte [4] del paquete maestro -> esclavo */
#define FLAG_ROUND_RESET 0x10u  /* redibujar campo tras gol */
#define FLAG_MASTER_EXIT 0x80u  /* maestro cancelo la partida */

#define PKT_SIZE 5

/* Bytes de identificacion durante el handshake */
#define HS_MASTER_BYTE 0xA5u
#define HS_SLAVE_BYTE  0x5Au

/* ── Utilidades ─────────────────────────────────────────────────────────── */
static int imin(int a, int b)              { return a < b ? a : b; }
static int imax(int a, int b)              { return a > b ? a : b; }
static int iclamp(int v, int lo, int hi)   { return v < lo ? lo : (v > hi ? hi : v); }
static void mp_delay(volatile int n)       { while (n--); }

/* ── Dibujo ─────────────────────────────────────────────────────────────── */

/* Dibuja la pelota como un bloque solido de BALL_SZ filas, alineado a palabra */
static void mp_draw_ball(int bx, int by, unsigned char color) {
    unsigned int w  = color_to_word(color);
    int          wx = bx / 8;
    for (int r = 0; r < BALL_SZ; r++)
        write_word((by + r) * WORDS_PER_LINE + wx, w);
}

/*
 * Borra la region que cubre la pelota entre su posicion anterior (px,py)
 * y la nueva (nx,ny). Se calcula el bounding box union y se escribe cero.
 * Si la posicion no cambio no hace nada.
 */
static void mp_erase_ball(int px, int py, int nx, int ny) {
    if (px == nx && py == ny) return;
    int left  = imin(px, nx);
    int right = imax(px, nx) + BALL_SZ;
    int top   = imax(imin(py, ny) - 1, 0);
    int bot   = imin(imax(py, ny) + BALL_SZ + 1, SCREEN_H);
    int words = (right - left) / 8;
    for (int row = top; row < bot; row++) {
        int base = row * WORDS_PER_LINE + left / 8;
        for (int w = 0; w < words; w++)
            write_word(base + w, 0);
    }
}

static void mp_draw_pad(int x, int y, unsigned char color) {
    draw_rect(x, y, PAD_W, PAD_H, color);
}

/*
 * Borra solo la franja descubierta al mover la paleta de oy a ny.
 * Evita redibujar toda la paleta cada frame.
 */
static void mp_erase_pad_strip(int x, int ny, int oy) {
    if      (ny > oy) draw_rect(x, oy,        PAD_W, ny - oy,  COLOR_BLACK);
    else if (ny < oy) draw_rect(x, ny + PAD_H, PAD_W, oy - ny, COLOR_BLACK);
}

/* Dibuja el marcador centrado en SCORE_Y */
static void mp_draw_score(int p1, int p2) {
    char buf[14];
    buf[0]='J'; buf[1]='1'; buf[2]=':'; buf[3]=' ';
    buf[4]='0'+(p1>9?9:p1);
    buf[5]=' '; buf[6]=' '; buf[7]=' ';
    buf[8]='J'; buf[9]='2'; buf[10]=':'; buf[11]=' ';
    buf[12]='0'+(p2>9?9:p2);
    buf[13]=0;
    ui_draw_text_centered(SCORE_Y, buf, COLOR_GRAY_LIGHT);
}

/* Redibuja el campo completo: fondo, marcador, paletas y pelota */
static void mp_draw_field(int my, int sy, int bx, int by, int p1, int p2) {
    clear_screen(COLOR_BLACK);
    mp_draw_score(p1, p2);
    mp_draw_pad(PLAYER_X, my, COLOR_YELLOW);
    mp_draw_pad(CPU_X,    sy, COLOR_CYAN_LIGHT);
    mp_draw_ball(bx, by, COLOR_WHITE);
}

/* Mueve la paleta segun los flags de direccion y la clampea al area de juego */
static void mp_move_pad(int *y, int up, int down) {
    if (up)   *y -= PAD_SPEED;
    if (down) *y += PAD_SPEED;
    *y = iclamp(*y, 0, SCREEN_H - PAD_H);
}

/*
 * Avanza la pelota un paso y gestiona rebotes en paredes y paletas.
 * Retorna  1 si salio por la derecha (punto para p1),
 *         -1 si salio por la izquierda (punto para p2),
 *          0 si la pelota sigue en juego.
 */
static int mp_step_ball(int *bx, int *by, int *vx, int *vy, int my, int sy) {
    *bx += *vx;
    *by += *vy;

    if (*by <= TOP_WALL)            { *by = TOP_WALL;            if (*vy < 0) *vy = -*vy; }
    if (*by + BALL_SZ >= SCREEN_H-1) { *by = SCREEN_H-BALL_SZ-1; if (*vy > 0) *vy = -*vy; }

    /* Colision con paleta izquierda (maestro) */
    if (*vx < 0 && *bx <= PLAYER_X+PAD_W && *bx+BALL_SZ > PLAYER_X &&
        *by+BALL_SZ > my && *by < my+PAD_H) {
        *bx = PLAYER_X+PAD_W;
        *vx = -*vx;
    }
    /* Colision con paleta derecha (esclavo) */
    if (*vx > 0 && *bx+BALL_SZ >= CPU_X && *bx < CPU_X+PAD_W &&
        *by+BALL_SZ > sy && *by < sy+PAD_H) {
        *bx = CPU_X-BALL_SZ;
        *vx = -*vx;
    }

    if (*bx + BALL_SZ <= 0) return -1;
    if (*bx >= SCREEN_W)    return  1;
    return 0;
}

/*
 * HANDSHAKE
 * El maestro envia HS_MASTER_BYTE hasta recibir HS_SLAVE_BYTE.
 * El esclavo espera SS activo y verifica el byte del maestro.
 * Retorna 1 si la conexion se establecio, 0 si se cancelo con BTNC.
 */
int pong_mp_handshake(int is_master) {

    if (is_master) {
        spi_init_master();
        clear_screen(COLOR_BLACK);
        ui_draw_text_centered(220, "MAESTRO: buscando esclavo...", COLOR_YELLOW);
        ui_draw_text_centered(280, "BTNC para cancelar", COLOR_GRAY_LIGHT);

        while (1) {
            if (Xil_In32(GPIO_BUTTONS_BASE + GPIO_DATA_REG) & BTN_C) return 0;

            Xil_Out32(SPI_DATA, 0u);                          /* SS activo */
            unsigned char rx = spi_master_transfer(HS_MASTER_BYTE);
            Xil_Out32(SPI_DATA, PIN_SS);                      /* SS inactivo */

            if (rx == HS_SLAVE_BYTE) {
                clear_screen(COLOR_BLACK);
                ui_draw_text_centered(220, "CONEXION ESTABLECIDA!", COLOR_GREEN_LIGHT);
                mp_delay(5000000);
                return 1;
            }
            mp_delay(200000);
        }

    } else {
        spi_init_slave();
        clear_screen(COLOR_BLACK);
        ui_draw_text_centered(220, "ESCLAVO: esperando maestro...", COLOR_CYAN_LIGHT);
        ui_draw_text_centered(280, "BTNC para cancelar", COLOR_GRAY_LIGHT);

        while (1) {
            if (Xil_In32(GPIO_BUTTONS_BASE + GPIO_DATA_REG) & BTN_C) return 0;
            if (Xil_In32(SPI_DATA) & PIN_SS) continue;  /* esperar SS activo */

            unsigned char rx = 0;
            int ok = spi_slave_transfer(HS_SLAVE_BYTE, &rx);

            /* Esperar SS inactivo; BTNC cancela si el maestro no lo sube */
            while (!(Xil_In32(SPI_DATA) & PIN_SS)) {
                if (Xil_In32(GPIO_BUTTONS_BASE + GPIO_DATA_REG) & BTN_C) return 0;
            }

            if (ok == 0 && rx == HS_MASTER_BYTE) {
                clear_screen(COLOR_BLACK);
                ui_draw_text_centered(220, "CONEXION ESTABLECIDA!", COLOR_GREEN_LIGHT);
                mp_delay(5000000);
                return 1;
            }
        }
    }
}

/* 
 * JUEGO MAESTRO
 * El maestro ejecuta toda la fisica y envia el estado al esclavo cada frame.
 * El esclavo responde con su posicion de paleta en el byte [0] del paquete.
*/

void pong_mp_master_run(void) {
    /* Debounce: esperar a que BTNC se suelte tras el handshake */
    while (Xil_In32(GPIO_BUTTONS_BASE + GPIO_DATA_REG) & BTN_C);

    static unsigned int frame_count = 0;

    int master_y = (SCREEN_H - PAD_H) / 2;
    int slave_y  = (SCREEN_H - PAD_H) / 2;
    int bx = ((SCREEN_W - BALL_SZ) / 2) & ~7;  /* alinear a palabra */
    int by = (SCREEN_H - BALL_SZ)  / 2;
    int vx = BALL_VX;
    int vy = ((frame_count >> 2) & 1) ? BALL_VY : -BALL_VY;
    int p1 = 0, p2 = 0;

    int prev_my = master_y, prev_sy = slave_y;
    int prev_bx = bx, prev_by = by;
    int game_over = 0, p1_wins = 0;
    int prev_exit = 0;

    /* Sincronizacion inicial: enviar estado y esperar primer respuesta valida
       del esclavo (slave_y_rx != 0) antes de iniciar la fisica */
       
    while (1) {
        Xil_Out32(SPI_DATA, 0u);

        unsigned char slave_y_rx = spi_master_transfer((unsigned char)(bx / 8));
        spi_master_transfer((unsigned char)(by / 2));
        spi_master_transfer((unsigned char)(master_y / 2));
        spi_master_transfer((unsigned char)(((p2 & 0xF) << 4) | (p1 & 0xF)));
        spi_master_transfer(0);

        Xil_Out32(SPI_DATA, PIN_SS);

        if (slave_y_rx != 0) {
            slave_y = iclamp((int)slave_y_rx * 2, 0, SCREEN_H - PAD_H);
            break;
        }
        mp_delay(50000);
    }

    mp_draw_field(master_y, slave_y, bx, by, p1, p2);

    volatile unsigned int tick = 0;

    while (1) {
        /* Esperar MP_TICK ciclos antes de procesar el siguiente frame */
        tick++;
        if (tick < MP_TICK) continue;
        tick = 0;
        frame_count++;

        prev_my = master_y;
        prev_sy = slave_y;
        prev_bx = bx;
        prev_by = by;

        unsigned int btn = Xil_In32(GPIO_BUTTONS_BASE + GPIO_DATA_REG);
        int exit_now = (btn & BTN_C) ? 1 : 0;
        int up       = (btn & BTN_U) ? 1 : 0;
        int down     = (btn & BTN_D) ? 1 : 0;

        /* Flanco de subida en BTNC: notificar al esclavo y salir */
        if (exit_now && !prev_exit) {
            unsigned char tx_exit[PKT_SIZE];
            tx_exit[0] = (unsigned char)(bx / 8);
            tx_exit[1] = (unsigned char)(by / 2);
            tx_exit[2] = (unsigned char)(master_y / 2);
            tx_exit[3] = (unsigned char)(((p2 & 0xF) << 4) | (p1 & 0xF));
            tx_exit[4] = FLAG_MASTER_EXIT;

            /* Enviar varias veces para asegurar que el esclavo lo reciba */
            for (int i = 0; i < 10; i++) {
                Xil_Out32(SPI_DATA, 0u);
                spi_master_transfer(tx_exit[0]);
                spi_master_transfer(tx_exit[1]);
                spi_master_transfer(tx_exit[2]);
                spi_master_transfer(tx_exit[3]);
                spi_master_transfer(tx_exit[4]);
                Xil_Out32(SPI_DATA, PIN_SS);
                mp_delay(50000);
            }

            clear_screen(COLOR_BLACK);
            return;
        }
        prev_exit = exit_now;

        if (!game_over) mp_move_pad(&master_y, up, down);

        int r = mp_step_ball(&bx, &by, &vx, &vy, master_y, slave_y);

        if (r != 0) {
            /* Gol: actualizar marcador y reiniciar pelota al centro */
            if (r ==  1) p1++;
            if (r == -1) p2++;

            bx = ((SCREEN_W - BALL_SZ) / 2) & ~7;
            by = (SCREEN_H - BALL_SZ) / 2;
            vx = (r == 1) ? BALL_VX : -BALL_VX;
            vy = ((frame_count >> 2) & 1) ? BALL_VY : -BALL_VY;
            prev_bx = bx; prev_by = by;

            /* Resincronizar esclavo tras el gol antes de continuar la fisica */
            
            while (1) {
                Xil_Out32(SPI_DATA, 0u);

                unsigned char slave_y_rx = spi_master_transfer((unsigned char)(bx / 8));
                spi_master_transfer((unsigned char)(by / 2));
                spi_master_transfer((unsigned char)(master_y / 2));
                spi_master_transfer((unsigned char)(((p2 & 0xF) << 4) | (p1 & 0xF)));
                spi_master_transfer(0);

                Xil_Out32(SPI_DATA, PIN_SS);

                if (slave_y_rx != 0) {
                    slave_y = iclamp((int)slave_y_rx * 2, 0, SCREEN_H - PAD_H);
                    break;
                }
                mp_delay(50000);
            }

            mp_draw_field(master_y, slave_y, bx, by, p1, p2);

            prev_my = master_y;
            prev_sy = slave_y;
            prev_bx = bx;
            prev_by = by;

            if (p1 >= WIN_SCORE || p2 >= WIN_SCORE) {
                p1_wins = (p1 >= WIN_SCORE) ? 1 : 0;
                if (p1_wins)
                    ui_draw_text_centered(220, "GANO JUGADOR 1!", COLOR_YELLOW);
                else
                    ui_draw_text_centered(220, "GANO JUGADOR 2!", COLOR_CYAN_LIGHT);
                ui_draw_text_centered(260, "PRESIONA BTNC PARA SALIR", COLOR_WHITE);

                /* Paquete de game_over con flag de ganador */
                
                unsigned char tx_go[PKT_SIZE];
                tx_go[0] = (unsigned char)(bx / 8);
                tx_go[1] = (unsigned char)(by / 2);
                tx_go[2] = (unsigned char)(master_y / 2);
                tx_go[3] = (unsigned char)(((p2 & 0xF) << 4) | (p1 & 0xF));
                tx_go[4] = 0x04u | (p1_wins ? 0x08u : 0u);

                /* Seguir enviando el estado de game_over al esclavo hasta que
                   el maestro presione BTNC */
                   
                while (1) {
                    unsigned int b2 = Xil_In32(GPIO_BUTTONS_BASE + GPIO_DATA_REG);

                    Xil_Out32(SPI_DATA, 0u);
                    spi_master_transfer(tx_go[0]);
                    spi_master_transfer(tx_go[1]);
                    spi_master_transfer(tx_go[2]);
                    spi_master_transfer(tx_go[3]);
                    spi_master_transfer(tx_go[4]);
                    Xil_Out32(SPI_DATA, PIN_SS);

                    mp_delay(50000);

                    if (b2 & BTN_C) {
                        game_over = 1;
                        break;
                    }
                }
                clear_screen(COLOR_BLACK);
                return;
            }

        } else {
            /* Frame normal: construir y enviar paquete, recibir posicion del esclavo */
            
            unsigned char tx[PKT_SIZE];
            tx[0] = (unsigned char)(bx / 8);
            tx[1] = (unsigned char)(by / 2);
            tx[2] = (unsigned char)(master_y / 2);
            tx[3] = (unsigned char)(((p2 & 0xF) << 4) | (p1 & 0xF));
            tx[4] = (unsigned char)((vx > 0 ? 0x01u : 0u) |
                                    (vy > 0 ? 0x02u : 0u) |
                                    (game_over ? 0x04u : 0u) |
                                    (p1_wins   ? 0x08u : 0u));

            Xil_Out32(SPI_DATA, 0u);
            unsigned char slave_y_rx = spi_master_transfer(tx[0]);
            spi_master_transfer(tx[1]);
            spi_master_transfer(tx[2]);
            spi_master_transfer(tx[3]);
            spi_master_transfer(tx[4]);
            Xil_Out32(SPI_DATA, PIN_SS);

            if (slave_y_rx != 0)
                slave_y = iclamp((int)slave_y_rx * 2, 0, SCREEN_H - PAD_H);

            /* Renderizado incremental: borrar solo la franja descubierta por
               cada paleta y el area union de la pelota, luego redibujar */
               
            mp_erase_pad_strip(PLAYER_X, master_y, prev_my);
            mp_erase_pad_strip(CPU_X,    slave_y,  prev_sy);
            mp_erase_ball(prev_bx, prev_by, bx, by);
            mp_draw_pad(PLAYER_X, master_y, COLOR_YELLOW);
            mp_draw_pad(CPU_X,    slave_y,  COLOR_CYAN_LIGHT);
            mp_draw_ball(bx, by, COLOR_WHITE);
        }
    }
}

/* 
 * JUEGO ESCLAVO
 * El esclavo recibe el estado completo del maestro cada frame via SPI y
 * responde con su posicion de paleta. No ejecuta fisica propia.
  */
  
void pong_mp_slave_run(void) {
    /* Debounce: esperar a que BTNC se suelte tras el handshake */
    while (Xil_In32(GPIO_BUTTONS_BASE + GPIO_DATA_REG) & BTN_C);

    int slave_y  = (SCREEN_H - PAD_H) / 2;
    int master_y = (SCREEN_H - PAD_H) / 2;
    int bx = ((SCREEN_W - BALL_SZ) / 2) & ~7;
    int by = (SCREEN_H - BALL_SZ)  / 2;
    int p1 = 0, p2 = 0;

    int prev_my = master_y, prev_sy = slave_y;
    int prev_bx = bx, prev_by = by;

    int field_drawn      = 0;  /* se vuelve 1 tras el primer dibujo completo */
    int slave_ready_drawn = 0; /* se vuelve 1 cuando el esclavo ya dibujo y puede enviar su y */

    int last_draw_p1 = p1;
    int last_draw_p2 = p2;

    while (1) {
        /* Esperar SS activo del maestro antes de iniciar la transferencia */
        if (Xil_In32(SPI_DATA) & PIN_SS) continue;

        unsigned int btn = Xil_In32(GPIO_BUTTONS_BASE + GPIO_DATA_REG);
        mp_move_pad(&slave_y, (btn & BTN_U) ? 1 : 0, (btn & BTN_D) ? 1 : 0);

        /* Enviar posicion de paleta solo si el campo ya esta dibujado;
           antes de eso enviar 0 para indicar que el esclavo aun no esta listo */
        unsigned char rx[PKT_SIZE] = {0, 0, 0, 0, 0};
        unsigned char tx0 = slave_ready_drawn ? (unsigned char)(slave_y / 2) : 0;

        int err = 0;
        err |= spi_slave_transfer(tx0, &rx[0]);
        err |= spi_slave_transfer(0,   &rx[1]);
        err |= spi_slave_transfer(0,   &rx[2]);
        err |= spi_slave_transfer(0,   &rx[3]);
        err |= spi_slave_transfer(0,   &rx[4]);

        /* Esperar SS inactivo antes de procesar el frame */
        while (!(Xil_In32(SPI_DATA) & PIN_SS));

        if (err) continue;  /* timeout en algun flanco: descartar frame */

        /* Decodificar paquete recibido del maestro */
        int new_bx       = (int)rx[0] * 8;
        int new_by       = (int)rx[1] * 2;
        int new_master_y = (int)rx[2] * 2;
        int new_p1       = rx[3] & 0x0F;
        int new_p2       = (rx[3] >> 4) & 0x0F;
        int flags        = rx[4];

        int master_exit = (flags & FLAG_MASTER_EXIT) ? 1 : 0;
        int game_over   = (flags & 0x04u) ? 1 : 0;
        int p1_wins     = (flags & 0x08u) ? 1 : 0;

        p1 = new_p1;
        p2 = new_p2;

        /* Detectar gol comparando marcador con el ultimo dibujado */
        int goal = (p1 != last_draw_p1 || p2 != last_draw_p2);

        if (master_exit) {
            clear_screen(COLOR_BLACK);
            ui_draw_text_centered(220, "MAESTRO DESCONECTADO", COLOR_RED_DARK);
            ui_draw_text_centered(260, "PRESIONA BTNC PARA SALIR", COLOR_WHITE);
            while (!(Xil_In32(GPIO_BUTTONS_BASE + GPIO_DATA_REG) & BTN_C));
            clear_screen(COLOR_BLACK);
            return;
        }

        if (game_over) {
            if (p1_wins)
                ui_draw_text_centered(220, "GANO JUGADOR 1!", COLOR_YELLOW);
            else
                ui_draw_text_centered(220, "GANO JUGADOR 2!", COLOR_CYAN_LIGHT);
            ui_draw_text_centered(260, "PRESIONA BTNC PARA SALIR", COLOR_WHITE);
            while (!(Xil_In32(GPIO_BUTTONS_BASE + GPIO_DATA_REG) & BTN_C));
            clear_screen(COLOR_BLACK);
            return;
        }

        /* Si la pelota salio del area visible y no hubo gol, actualizar estado
           sin renderizar para evitar artefactos en el frame de transicion */
        int ball_out = (new_bx + BALL_SZ <= 0) || (new_bx >= SCREEN_W);
        if (ball_out && !goal) {
            prev_bx = new_bx; prev_by = new_by;
            prev_my = new_master_y; prev_sy = slave_y;
            bx = new_bx; by = new_by; master_y = new_master_y;
            continue;
        }

        /* Primer frame: dibujo completo del campo */
        if (!field_drawn) {
            clear_screen(COLOR_BLACK);
            mp_draw_score(p1, p2);
            mp_draw_pad(PLAYER_X, new_master_y, COLOR_YELLOW);
            mp_draw_pad(CPU_X,    slave_y,      COLOR_CYAN_LIGHT);
            mp_draw_ball(new_bx, new_by, COLOR_WHITE);

            field_drawn       = 1;
            slave_ready_drawn = 1;
            last_draw_p1 = p1;
            last_draw_p2 = p2;

            prev_bx = new_bx; prev_by = new_by;
            prev_my = new_master_y; prev_sy = slave_y;
            bx = new_bx; by = new_by; master_y = new_master_y;
            continue;
        }

        /* Tras un gol: redibujar campo completo y actualizar marcador */
        if (goal) {
            draw_rect(PLAYER_X, prev_my, PAD_W, PAD_H, COLOR_BLACK);
            draw_rect(CPU_X,    prev_sy, PAD_W, PAD_H, COLOR_BLACK);
            mp_erase_ball(prev_bx, prev_by, new_bx, new_by);

            mp_draw_score(p1, p2);
            mp_draw_pad(PLAYER_X, new_master_y, COLOR_YELLOW);
            mp_draw_pad(CPU_X,    slave_y,      COLOR_CYAN_LIGHT);
            mp_draw_ball(new_bx, new_by, COLOR_WHITE);

            last_draw_p1 = p1;
            last_draw_p2 = p2;

            prev_bx = new_bx; prev_by = new_by;
            prev_my = new_master_y; prev_sy = slave_y;
            bx = new_bx; by = new_by; master_y = new_master_y;
            continue;
        }

        /* Frame normal: renderizado incremental */
        mp_erase_pad_strip(PLAYER_X, new_master_y, prev_my);
        mp_erase_pad_strip(CPU_X,    slave_y,       prev_sy);
        mp_erase_ball(prev_bx, prev_by, new_bx, new_by);
        mp_draw_pad(PLAYER_X, new_master_y, COLOR_YELLOW);
        mp_draw_pad(CPU_X,    slave_y,      COLOR_CYAN_LIGHT);
        mp_draw_ball(new_bx, new_by, COLOR_WHITE);

        prev_bx = new_bx; prev_by = new_by;
        prev_my = new_master_y; prev_sy = slave_y;
        bx = new_bx; by = new_by; master_y = new_master_y;
    }
}
