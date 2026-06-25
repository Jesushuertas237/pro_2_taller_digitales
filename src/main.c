#include "vga.h"
#include "font.h"
#include "sd_image.h"
#include "input.h"
#include "ui_text.h"
#include <string.h>

/*
 * Game State Machine
 * STATE_MENU (MENU.BIN background + prompts, SW0/SW1 transitions)
 * STATE_SELECT_MASTER (TWO.BIN background, SW15 selects)
 * STATE_PRESS_START (TWO.BIN background, BTNC to start)
 * STATE_GAME (black background, BTNC returns to MENU)
 */

typedef enum {
	STATE_MENU,
	STATE_SELECT_MASTER,
	STATE_PRESS_START,
	STATE_GAME
} GameState;

/* Input bit mapping for Nexys A7. SW_N -> bit N standard. BTNC_BIT assumed at bit 0. */
#define SW0_BIT   0
#define SW1_BIT   1
#define SW15_BIT  15
#define BTNC_BIT  0

static int bit_is_set(unsigned int reg, int bit) {
	return (reg >> bit) & 0x1;
}

/*
 * Image cache in DDR2. sd_image_load() is slow (SPI byte-by-byte reads).
 * Preload both images at startup to fixed DDR2 addresses to avoid per-state latency.
 * Cache addresses chosen to avoid conflicts with sd_image_load() temp buffer (0x80100000)
 * and vga_cache (0x81000000).
 */
#define IMG_BYTES        (480 * 480 / 2)
#define MENU_CACHE_ADDR  0x81100000
#define TWO_CACHE_ADDR   0x81200000

static unsigned char * const menu_cache = (unsigned char *)MENU_CACHE_ADDR;
static unsigned char * const two_cache  = (unsigned char *)TWO_CACHE_ADDR;
static int menu_ok = 0;
static int two_ok  = 0;

static void preload_images(void) {
	if (sd_image_load("MENU.BIN") == 0) {
		memcpy(menu_cache, sd_image_get_buffer(), IMG_BYTES);
		menu_ok = 1;
	}
	if (sd_image_load("TWO.BIN") == 0) {
		memcpy(two_cache, sd_image_get_buffer(), IMG_BYTES);
		two_ok = 1;
	}
}

/* State-specific rendering. Text, color, positioning defined here. ui_text.c provides generic primitive. */

static void draw_state_menu(void) {
	if (menu_ok) {
		draw_image_centered(menu_cache);
	} else {
		clear_screen(COLOR_MAGENTA_LIGHT);  /* Fallback if MENU.BIN load failed */
	}

	/* Use "\xBF" (¿ in single byte). UTF-8 encoding would break font table. */
	ui_draw_text_centered(390, "\xBF EN QUE MODO QUIERES JUGAR?", COLOR_RED_LIGHT);
	ui_draw_text_centered(440, "SW0(SINGLE PLAYER)     SW1(MULTIPLAYER)", COLOR_CYAN_LIGHT);
}

static void draw_state_select_master(void) {
	if (two_ok) {
		draw_image_centered(two_cache);
	} else {
		clear_screen(COLOR_MAGENTA_LIGHT);
	}
	ui_draw_text_centered(253, "SELECCIONA EL MAESTRO CON SW15", COLOR_RED_LIGHT);
}

static void draw_state_press_start(void) {
	if (two_ok) {
		draw_image_centered(two_cache);
	} else {
		clear_screen(COLOR_MAGENTA_LIGHT);
	}
	ui_draw_text_centered(253, "PRESIONE BTNC PARA INICIAR", COLOR_RED_LIGHT);
}

static void draw_state_game(void) {
	/* Placeholder game state (black background + text). Full game logic goes here. */
	clear_screen(COLOR_BLACK);
	ui_draw_text_centered(230, "JUEGO EN PROCESO", COLOR_WHITE);
}

int main(void) {
	input_init();
	sd_image_init();
	preload_images();  /* Load images from SD once at startup */

	GameState state = STATE_MENU;
	GameState prev_state = (GameState)(-1);  /* Force first render */

	int sw0_prev = 0, sw1_prev = 0, sw15_prev = 0, btnc_prev = 0;

	while (1) {
		/* Render only when state changes to minimize VGA writes and image reloads */
		if (state != prev_state) {
			switch (state) {
				case STATE_MENU:          draw_state_menu(); break;
				case STATE_SELECT_MASTER: draw_state_select_master(); break;
				case STATE_PRESS_START:   draw_state_press_start(); break;
				case STATE_GAME:          draw_state_game(); break;
			}
			prev_state = state;
		}

		unsigned int sw  = input_read_switches();
		unsigned int btn = input_read_buttons();

		int sw0_now  = bit_is_set(sw, SW0_BIT);
		int sw1_now  = bit_is_set(sw, SW1_BIT);
		int sw15_now = bit_is_set(sw, SW15_BIT);
		int btnc_now = bit_is_set(btn, BTNC_BIT);

		/* Rising edges only (0->1). Prevents repeated transitions if input held. */
		int sw0_edge  = sw0_now  && !sw0_prev;
		int sw1_edge  = sw1_now  && !sw1_prev;
		int sw15_edge = sw15_now && !sw15_prev;
		int btnc_edge = btnc_now && !btnc_prev;

		sw0_prev  = sw0_now;
		sw1_prev  = sw1_now;
		sw15_prev = sw15_now;
		btnc_prev = btnc_now;

		switch (state) {
			case STATE_MENU:
				if (sw0_edge) {
					state = STATE_GAME;
				} else if (sw1_edge) {
					state = STATE_SELECT_MASTER;
				}
				break;

			case STATE_SELECT_MASTER:
				if (sw15_edge) {
					state = STATE_PRESS_START;
				}
				break;

			case STATE_PRESS_START:
				if (btnc_edge) {
					state = STATE_GAME;
				}
				break;

			case STATE_GAME:
				if (btnc_edge) {
					state = STATE_MENU;
				}
				break;
		}
	}

	return 0;
}