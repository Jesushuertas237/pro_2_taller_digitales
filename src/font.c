#include "font.h"
#include "vga.h"
#include <stdint.h>

/*
 * Font Bitmap Table (42 glyphs, 8x8 pixels each)
 * Storage: DDR2 section (.images)
 * Bit order: byte[row] = 8 pixels, bit 7 (left) to bit 0 (right)
 * Character indices: A-Z(0-25), 0-9(26-35), space(36), dot(37), ¿?(38), ?(39), ((40), )(41)
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
	/* . */ {0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x00},
	/* ¿ (38) */ {0x18, 0x00, 0x18, 0x30, 0x40, 0x42, 0x3C, 0x00},
	/* ?  (39) */ {0x3C, 0x42, 0x02, 0x0C, 0x18, 0x00, 0x18, 0x00},
	/* (  (40) */ {0x0C, 0x10, 0x20, 0x20, 0x20, 0x10, 0x0C, 0x00},
	/* )  (41) */ {0x30, 0x08, 0x04, 0x04, 0x04, 0x08, 0x30, 0x00},
};

/* ASCII to bitmap index mapping. Returns -1 for unsupported characters. */
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
 * Renders glyph as 8-pixel words aligned to word boundary.
 * Format: 4 bits per pixel, pixel [7..0] -> bits [31..0] (rightmost pixel LSB)
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
				int nibble_pos = (5 - col) & 7;
				word_value |= (color_nibble << (nibble_pos * 4));
			}
		}

		int word_addr = (y + row) * WORDS_PER_LINE + (x / 8);
		write_word(word_addr, word_value);
	}
}

/* Renders null-terminated string starting at (x,y), 8 pixels per character. */
void draw_string(int x, int y, const char *str, unsigned char color) {
	int px = x;
	while (*str) {
		draw_char(px, y, *str, color);
		px += 8;
		str++;
	}
}

/*
 * Magnified glyph rendering using draw_rect() for each pixel.
 * Each glyph pixel expands to FONT_BLOCK_W x vscale pixels.
 * Avoids nibble-mixing by writing uniform 8-pixel blocks.
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
			int bit = FONT_MIRROR ? sx : (7 - sx);
			if (byte & (1 << bit)) {
				/* Bloque de 8px de ancho => draw_rect escribe palabras uniformes */
				draw_rect(x + sx * FONT_BLOCK_W, y + row * vscale,
				          FONT_BLOCK_W, vscale, color);
			}
		}
	}
}

void draw_string_big(int x, int y, const char *str, unsigned char color, int vscale) {
	int px = x;
	int advance = 8 * FONT_BLOCK_W + FONT_BLOCK_W;  /* Character width including spacing */
	while (*str) {
		draw_char_big(px, y, *str, color, vscale);
		px += advance;
		str++;
	}
}

/* ── Experiment: Compact draw_char with nibble shifting──────────── */
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

/* ── Experiment: shift + optional mirror ─────────────────────────── */
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