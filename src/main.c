#include "vga.h"
#include "font.h"
#include "sd_image.h"
#include "input.h"
#include "ui_text.h"
#include <string.h>

/* ============================================================================
 * MAQUINA DE ESTADOS PRINCIPAL DEL JUEGO
 * ============================================================================
 *
 *   MENU (portada MENU.BIN + pregunta + SW0/SW1)
 *     --SW0--> GAME
 *     --SW1--> SELECT_MASTER
 *
 *   SELECT_MASTER (fondo TWO.BIN, "SELECCIONA EL MAESTRO CON SW15")
 *     --SW15--> PRESS_START
 *
 *   PRESS_START (fondo TWO.BIN, "PRESIONE BTNC PARA INICIAR")
 *     --BTNC--> GAME
 *
 *   GAME (fondo negro -- placeholder, sin imagen, "JUEGO EN PROCESO")
 *     --BTNC--> MENU
 * ============================================================================ */

typedef enum {
	STATE_MENU,
	STATE_SELECT_MASTER,
	STATE_PRESS_START,
	STATE_GAME
} GameState;

/* ----------------------------------------------------------------------------
 * Mapeo de bits de entrada.
 *
 * SW0/SW1/SW15 asumen el mapeo estandar de la Nexys A7 (switch N -> bit N).
 * BTNC_BIT es una SUPOSICION (bit 0 del registro de botones) -- verifica
 * la posicion real de BTNC en tu vector de botones (.xdc / input.c) y
 * ajustala aqui si no coincide.
 * ---------------------------------------------------------------------- */
#define SW0_BIT   0
#define SW1_BIT   1
#define SW15_BIT  15
#define BTNC_BIT  0   /* VERIFICAR */

static int bit_is_set(unsigned int reg, int bit) {
	return (reg >> bit) & 0x1;
}

/* ----------------------------------------------------------------------------
 * Cache de imagenes en DDR2. sd_image_load() es lento (lee la SD por SPI a
 * pulso, byte a byte, ~225 sectores por imagen) -- si lo llamaramos cada
 * vez que se entra a un estado, cada cambio de pantalla se sentiria con
 * latencia. En vez de eso, ambas imagenes se cargan UNA sola vez al
 * arrancar (preload_images(), llamada antes del while(1)) y se copian de
 * la SD a direcciones FIJAS en DDR2; de ahi en adelante draw_state_*()
 * solo dibuja desde esas direcciones, sin volver a tocar la SD.
 *
 * Direcciones elegidas bien lejos de DDR2_IMAGE_BASE (0x80100000, que
 * sd_image_load() usa solo de forma temporal durante la copia) y del
 * vga_cache (0x81000000), para no pisar nada.
 * ---------------------------------------------------------------------- */
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

/* ----------------------------------------------------------------------------
 * Dibujo de cada estado. Todo lo especifico de la pantalla (texto, color,
 * posicion) vive aqui -- ui_text.c solo da la primitiva generica de dibujo.
 * ---------------------------------------------------------------------- */

static void draw_state_menu(void) {
	if (menu_ok) {
		draw_image_centered(menu_cache);
	} else {
		clear_screen(COLOR_MAGENTA_LIGHT);  /* error cargando MENU.BIN */
	}

	/* "\xBF" = ¿ en un solo byte. NO escribir el caracter ¿ directo en el
	 * string -- en UTF-8 ocupa 2 bytes y rompe la tabla de font.c. */
	ui_draw_text_centered(390, "\xBF EN QUE MODO QUIERES JUGAR?", COLOR_RED_LIGHT);
	ui_draw_text_centered(440, "SW0(SINGLE PLAYER)     SW1(MULTIPLAYER)", COLOR_CYAN_LIGHT);
}

static void draw_state_select_master(void) {
	if (two_ok) {
		draw_image_centered(two_cache);
	} else {
		clear_screen(COLOR_MAGENTA_LIGHT);  /* error cargando TWO.BIN */
	}
	ui_draw_text_centered(253, "SELECCIONA EL MAESTRO CON SW15", COLOR_RED_LIGHT);
}

static void draw_state_press_start(void) {
	if (two_ok) {
		draw_image_centered(two_cache);
	} else {
		clear_screen(COLOR_MAGENTA_LIGHT);  /* error cargando TWO.BIN */
	}
	ui_draw_text_centered(253, "PRESIONE BTNC PARA INICIAR", COLOR_RED_LIGHT);
}

static void draw_state_game(void) {
	/* TODO: aqui entra la grafica/logica real del juego mas adelante.
	 * Por ahora, placeholder de fondo negro + texto, como se pidio. */
	clear_screen(COLOR_BLACK);
	ui_draw_text_centered(230, "JUEGO EN PROCESO", COLOR_WHITE);
}

int main(void) {
	input_init();
	sd_image_init();
	preload_images();  /* unica vez que se lee la SD en toda la ejecucion */

	GameState state = STATE_MENU;
	GameState prev_state = (GameState)(-1);  /* fuerza el primer dibujado */

	int sw0_prev = 0, sw1_prev = 0, sw15_prev = 0, btnc_prev = 0;

	while (1) {
		/* Redibuja SOLO cuando cambia de estado -- no en cada vuelta del
		 * loop, para no recargar la imagen ni reescribir VGA sin necesidad */
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

		/* Flancos de subida (0->1): asi un switch que ya estaba encendido
		 * desde antes (por ejemplo al volver al MENU) no dispara la
		 * transicion de nuevo solo; hay que volver a moverlo. */
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