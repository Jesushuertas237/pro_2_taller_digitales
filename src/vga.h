#ifndef VGA_H
#define VGA_H

#include "xil_io.h"

/* Direccion base del IP VGA y dimensiones de pantalla */
#define VGA_BASE       0xC0000000
#define SCREEN_W       640
#define SCREEN_H       480

/* Formato de framebuffer: 4bpp, 8 pixeles por palabra de 32 bits */
#define WORDS_PER_LINE 80       /* 640 / 8 */
#define TOTAL_WORDS    38400    /* 80 * 480 */

/* Paleta de 16 colores (indice de 4 bits) */
#define COLOR_BLACK         0x0
#define COLOR_BLUE_DARK     0x1
#define COLOR_GREEN_DARK    0x2
#define COLOR_CYAN_DARK     0x3
#define COLOR_RED_DARK      0x4
#define COLOR_MAGENTA_DARK  0x5
#define COLOR_ORANGE        0x6
#define COLOR_GRAY_LIGHT    0x7
#define COLOR_GRAY_DARK     0x8
#define COLOR_BLUE_LIGHT    0x9
#define COLOR_GREEN_LIGHT   0xA
#define COLOR_CYAN_LIGHT    0xB
#define COLOR_RED_LIGHT     0xC
#define COLOR_MAGENTA_LIGHT 0xD
#define COLOR_YELLOW        0xE
#define COLOR_WHITE         0xF

void write_word(int word_addr, unsigned int value);
unsigned int read_word(int word_addr);
unsigned int color_to_word(unsigned char color);
void clear_screen(unsigned char color);
void draw_pixel(int x, int y, unsigned char color);
void draw_hline(int x, int y, int w, unsigned char color);
void draw_vline(int x, int y, int h, unsigned char color);
void draw_rect(int x, int y, int w, int h, unsigned char color);
void draw_border(unsigned char color);
void delay_ms(int ms);

/* Copia imagen 480x480 a 4bpp desde DDR2 centrada en pantalla; rellena con negro a los lados */
void draw_image_centered(unsigned char *image_data);

#endif
