#include "vga.h"
#include "font.h"
#include "ui_text.h"

/* Dibuja text centrado horizontalmente en la fila y.
   El ancho se calcula asumiendo 8 pixeles por caracter (fuente fija). */
void ui_draw_text_centered(int y, const char *text, unsigned char color) {
    int n = 0;
    while (text[n]) n++;
    int w = n * 8;
    int x = (SCREEN_W - w) / 2;
    if (x < 0) x = 0;   /* evitar coordenada negativa si el texto excede el ancho */
    draw_string(x, y, text, color);
}
