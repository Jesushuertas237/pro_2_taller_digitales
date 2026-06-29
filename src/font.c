#include "font.h"
#include "vga.h"
#include <stdint.h>

/*
 * Tabla de bitmaps de la fuente (42 glifos, 8x8 pixeles cada uno).
 * Se ubica en la seccion .images (DDR2), separada del segmento de datos.
 * Orden de bits: bit 7 = columna izquierda, bit 0 = columna derecha.
 * Indices: A-Z (0-25), 0-9 (26-35), espacio (36), punto (37),
 *          ¿ (38), ? (39), ( (40), ) (41)
 */
static unsigned char font_bitmap[42][8] __attribute__((section(".images"))) = {
	/* A */ {0x3C, 0x42, 0x42, 0x7E, 0x42, 0x42, 0x42, 0x00},
	/* B */ {0x7C, 0x42, 0x42, 0x7C, 0x42, 0x42, 0x7C, 0x00},
	/* C */ {0x3C, 0x42, 0x40, 0x40, 0x40, 0x42, 0x3C, 0x00},
	/* D */ {0x7C, 0x42, 0x42, 0x42, 0x42, 0x42, 0x7C, 0x00},
	/* E */ {0x7E, 0x40, 0x40, 0x7C, 0x40, 0x40, 0x7E, 0x00},
	/* F */ {0x7E, 0x40, 0x40, 0x7C, 0x40, 0x40, 0x40, 0x00},
	/* G */ {0x3C, 0x42, 0x40, 0x4E, 0x42, 0x42, 0x3C, 0x00},
	/* H */ {0x42, 0x42, 0x42, 0x7E, 0x42, 0x42, 0x42, 0x00},
	/* I */ {0x7E, 0x18, 0x18, 0x18, 0x18, 0x18, 0x7E, 0x00},
	/* J */ {0x1E, 0x04, 0x04, 0x04, 0x44, 0x44, 0x38, 0x00},
	/* K */ {0x42, 0x44, 0x48, 0x70, 0x48, 0x44, 0x42, 0x00},
	/* L */ {0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x7E, 0x00},
	/* M */ {0x42, 0x66, 0x5A, 0x42, 0x42, 0x42, 0x42, 0x00},
	/* N */ {0x42, 0x62, 0x52, 0x4A, 0x46, 0x42, 0x42, 0x00},
	/* O */ {0x3C, 0x42, 0x42, 0x42, 0x42, 0x42, 0x3C, 0x00},
	/* P */ {0x7C, 0x42, 0x42, 0x7C, 0x40, 0x40, 0x40, 0x00},
	/* Q */ {0x3C, 0x42, 0x42, 0x42, 0x4A, 0x44, 0x3A, 0x00},
	/* R */ {0x7C, 0x42, 0x42, 0x7C, 0x48, 0x44, 0x42, 0x00},
	/* S */ {0x3C, 0x42, 0x40, 0x3C, 0x02, 0x42, 0x3C, 0x00},
	/* T */ {0x7E, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x00},
	/* U */ {0x42, 0x42, 0x42, 0x42, 0x42, 0x42, 0x3C, 0x00},
	/* V */ {0x42, 0x42, 0x42, 0x42, 0x42, 0x24, 0x18, 0x00},
	/* W */ {0x42, 0x42, 0x42, 0x42, 0x5A, 0x66, 0x42, 0x00},
	/* X */ {0x42, 0x24, 0x18, 0x18, 0x18, 0x24, 0x42, 0x00},
	/* Y */ {0x42, 0x24, 0x18, 0x18, 0x18, 0x18, 0x18, 0x00},
	/* Z */ {0x7E, 0x04, 0x08, 0x10, 0x20, 0x40, 0x7E, 0x00},
	/* 0 */ {0x3C, 0x42, 0x42, 0x42, 0x42, 0x42, 0x3C, 0x00},
	/* 1 */ {0x08, 0x18, 0x08, 0x08, 0x08, 0x08, 0x3E, 0x00},
	/* 2 */ {0x3C, 0x42, 0x02, 0x0C, 0x10, 0x20, 0x7E, 0x00},
	/* 3 */ {0x3C, 0x42, 0x02, 0x1C, 0x02, 0x42, 0x3C, 0x00},
	/* 4 */ {0x04, 0x0C, 0x14, 0x24, 0x7E, 0x04, 0x04, 0x00},
	/* 5 */ {0x7E, 0x40, 0x40, 0x7C, 0x02, 0x42, 0x3C, 0x00},
	/* 6 */ {0x1C, 0x20, 0x40, 0x7C, 0x42, 0x42, 0x3C, 0x00},
	/* 7 */ {0x7E, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x00},
	/* 8 */ {0x3C, 0x42, 0x42, 0x3C, 0x42, 0x42, 0x3C, 0x00},
	/* 9 */ {0x3C, 0x42, 0x42, 0x3E, 0x02, 0x42, 0x3C, 0x00},
	/* space */ {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	/* . */     {0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x00},
	/* ¿ (38) */ {0x18, 0x00, 0x18, 0x30, 0x40, 0x42, 0x3C, 0x00},
	/* ?  (39) */ {0x3C, 0x42, 0x02, 0x0C, 0x18, 0x00, 0x18, 0x00},
	/* (  (40) */ {0x0C, 0x10, 0x20, 0x20, 0x20, 0x10, 0x0C, 0x00},
	/* )  (41) */ {0x30, 0x08, 0x04, 0x04, 0x04, 0x08, 0x30, 0x00},
};

/*
 * Retorna el indice en font_bitmap[] para el caracter dado.
 * Mayusculas y minusculas comparten el mismo glifo.
 * Retorna -1 si el caracter no esta soportado.
 */
static int char_to_index(char c) {
	if (c >= 'A' && c <= 'Z') return c - 'A';
	if (c >= 'a' && c <= 'z') return c - 'a';
	if (c >= '0' && c <= '9') return 26 + (c - '0');
	if (c == ' ') return 36;
	if (c == '.') return 37;
	if (c == (char)0xBF) return 38;  /* ¿  -- usar "\xBF" en el string, no el caracter literal (ver nota abajo) */
	if (c == '?') return 39;
	if (c == '(') return 40;
	if (c == ')') return 41;
	return -1;
}

/*
 * Dibuja un glifo 8x8 en (x, y) del framebuffer VGA.
 * Formato de salida: 4 bits por pixel (16 colores).
 * Cada palabra de 32 bits contiene 8 pixeles; nibble mas alto = pixel izquierdo.
 * x debe ser multiplo de 8 para alinearse a palabra sin solapar vecinos.
 */
void draw_char(int x, int y, char c, unsigned char color) {
	int idx = char_to_index(c);
	if (idx < 0) return;

	unsigned char *glyph = font_bitmap[idx];
	uint32_t color_nibble = (color & 0xF);

	for (int row = 0; row < CHAR_HEIGHT; row++) {
		unsigned char byte = glyph[row];
		uint32_t word_value = 0;

		for (int col = 0; col < 8; col++) {
			if (byte & (1 << col)) {
				/* (5 - col) & 7 invierte el orden de columnas para que
				   el bit 7 del bitmap quede en el nibble mas alto de la palabra */
				int nibble_pos = (5 - col) & 7;
				word_value |= (color_nibble << (nibble_pos * 4));
			}
		}

		int word_addr = (y + row) * WORDS_PER_LINE + (x / 8);
		write_word(word_addr, word_value);
	}
}

/* Dibuja la cadena str a partir de (x, y), avanzando 8 pixeles por caracter. */
void draw_string(int x, int y, const char *str, unsigned char color) {
	int px = x;
	while (*str) {
		draw_char(px, y, *str, color);
		px += 8;
		str++;
	}
}

/*
 * Dibuja un glifo ampliado usando draw_rect() por cada pixel activo.
 * Cada pixel del bitmap se expande a FONT_BLOCK_W x vscale pixeles.
 * Al usar bloques de 8px de ancho se evita mezcla de nibbles entre pixeles.
 * FONT_MIRROR != 0 invierte el glifo horizontalmente.
 */
#define FONT_BLOCK_W 8
#define FONT_MIRROR  0

void draw_char_big(int x, int y, char c, unsigned char color, int vscale) {
	int idx = char_to_index(c);
	if (idx < 0) return;
	if (vscale < 1) vscale = 1;

	x = (x / 8) * 8;  /* alinear a palabra: cada bloque cae en palabras enteras */
	unsigned char *glyph = font_bitmap[idx];

	for (int row = 0; row < CHAR_HEIGHT; row++) {
		unsigned char byte = glyph[row];
		for (int sx = 0; sx < 8; sx++) {
			/* selecciona el bit segun modo espejo */
			int bit = FONT_MIRROR ? sx : (7 - sx);
			if (byte & (1 << bit)) {
				/* Bloque de 8px de ancho => draw_rect escribe palabras uniformes */
				draw_rect(x + sx * FONT_BLOCK_W, y + row * vscale,
				          FONT_BLOCK_W, vscale, color);
			}
		}
	}
}

/* Igual que draw_string pero usando glifos ampliados. El avance incluye
   un bloque de separacion entre caracteres. */
void draw_string_big(int x, int y, const char *str, unsigned char color, int vscale) {
	int px = x;
	int advance = 8 * FONT_BLOCK_W + FONT_BLOCK_W;  /* Character width including spacing */
	while (*str) {
		draw_char_big(px, y, *str, color, vscale);
		px += advance;
		str++;
	}
}

/*
 * Experimento: draw_char con corrimiento circular de nibbles.
 * rot desplaza la posicion de cada pixel dentro de la palabra de 32 bits:
 *   nibble_pos = (col + rot) & 7
 * Util para probar corrimientos horizontales sub-palabra sin mover la direccion
 * de escritura al framebuffer.
 */
void draw_char_rot(int x, int y, char c, unsigned char color, int rot) {
	int idx = char_to_index(c);
	if (idx < 0) return;

	x = (x / 8) * 8;
	unsigned char *glyph = font_bitmap[idx];
	uint32_t color_nibble = (color & 0xF);

	for (int row = 0; row < CHAR_HEIGHT; row++) {
		unsigned char byte = glyph[row];
		uint32_t word_value = 0;

		for (int col = 0; col < 8; col++) {
			if (byte & (1 << col)) {
				int nibble_pos = (col + rot) & 7;  /* corrimiento circular */
				word_value |= (color_nibble << (nibble_pos * 4));
			}
		}

		int word_addr = (y + row) * WORDS_PER_LINE + (x / 8);
		write_word(word_addr, word_value);
	}
}

/*
 * Experimento: corrimiento circular mas espejo horizontal opcional.
 * mirror=0: nibble_pos = (rot + col) & 7  (orden normal)
 * mirror=1: nibble_pos = (rot - col) & 7  (reflejo horizontal)
 * La mascara & 7 garantiza resultado valido con aritmetica modular.
 */
void draw_char_xform(int x, int y, char c, unsigned char color, int rot, int mirror) {
	int idx = char_to_index(c);
	if (idx < 0) return;

	x = (x / 8) * 8;
	unsigned char *glyph = font_bitmap[idx];
	uint32_t color_nibble = (color & 0xF);

	for (int row = 0; row < CHAR_HEIGHT; row++) {
		unsigned char byte = glyph[row];
		uint32_t word_value = 0;

		for (int col = 0; col < 8; col++) {
			if (byte & (1 << col)) {
				int base = mirror ? (rot - col) : (rot + col);
				int nibble_pos = base & 7;  /* circular */
				word_value |= (color_nibble << (nibble_pos * 4));
			}
		}

		int word_addr = (y + row) * WORDS_PER_LINE + (x / 8);
		write_word(word_addr, word_value);
	}
}
