/*
 * font.h - Modulo de renderizado de texto sobre VGA (640x480, fuente 8x8).
 * Depende de vga.h
 */
#ifndef FONT_H
#define FONT_H

#include "xil_types.h"

#define CHAR_WIDTH  8
#define CHAR_HEIGHT 8

/* Dibuja un caracter en (x, y). color es un indice de 4 bits (0x0-0xF). */
void draw_char(int x, int y, char c, unsigned char color);

/* Dibuja la cadena str a partir de (x, y), avanzando 8 pixeles por caracter. */
void draw_string(int x, int y, const char *str, unsigned char color);

/* Dibuja un caracter ampliado. Cada pixel del glifo se expande a 8 x vscale pixeles.
   Usa draw_rect() para evitar mezcla de nibbles en el IP VGA. */
void draw_char_big(int x, int y, char c, unsigned char color, int vscale);

/* Igual que draw_string pero con glifos ampliados segun vscale. */
void draw_string_big(int x, int y, const char *str, unsigned char color, int vscale);

/* Variante de draw_char con corrimiento circular de nibbles dentro de la palabra.
   rot en rango 0-7; rot=0 equivale al comportamiento estandar. */
void draw_char_rot(int x, int y, char c, unsigned char color, int rot);

/* Variante de draw_char_rot con espejo horizontal opcional.
   mirror=0: orden normal, mirror=1: reflejo horizontal del glifo. */
void draw_char_xform(int x, int y, char c, unsigned char color, int rot, int mirror);

#endif
