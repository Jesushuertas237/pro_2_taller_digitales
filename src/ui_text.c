#include "vga.h"
#include "font.h"
#include "ui_text.h"

void ui_draw_text_centered(int y, const char *text, unsigned char color) {
	int n = 0;
	while (text[n]) n++;

	int w = n * 8;  /* String width: 8 pixels per character */
	int x = (SCREEN_W - w) / 2;
	if (x < 0) x = 0;

	draw_string(x, y, text, color);
}
