#ifndef UI_TEXT_H
#define UI_TEXT_H

/* Dibuja text centrado horizontalmente en la fila y.
   color es un indice de 4 bits (0x0-0xF). */
void ui_draw_text_centered(int y, const char *text, unsigned char color);

#endif
