#include "vga.h"
#include <stdint.h>

/*
 * Cache de pantalla en DDR2 (0x81000000).
 * Evita leer directamente del hardware VGA, que cuelga en read_word.
 * Permite hacer RMW sobre el cache y luego escribir al hardware.
 * Tamaño: 640x480 / 8 pixeles por word * 4 bytes = 153.6 KB.
 * Se usa uint32_t explicitamente para garantizar 32 bits en cualquier ABI.
 */
static uint32_t *vga_cache = (uint32_t *)0x81000000; // antes 0x80000000

/* Escribe una palabra de 32 bits (8 pixeles a 4bpp) en el cache y en el hardware VGA */
void write_word(int word_addr, unsigned int value) {
    if (word_addr < TOTAL_WORDS) {
        vga_cache[word_addr] = (uint32_t)value;
        Xil_Out32(VGA_BASE + (word_addr << 2), (uint32_t)value);
    }
}

/* Lee una palabra del cache; evita acceder al hardware VGA directamente */
unsigned int read_word(int word_addr) {
    if (word_addr < TOTAL_WORDS) {
        return (unsigned int)vga_cache[word_addr];
    }
    return 0;
}

/*
 * Modifica un solo nibble (un pixel) dentro de la palabra correspondiente
 * sin alterar los otros 7 pixeles. Usa RMW sobre el cache.
 * nibble_pos: 7 = pixel izquierdo (x%8==0), 0 = pixel derecho.
 */
void draw_pixel(int x, int y, unsigned char color) {
    if (x < 0 || x >= SCREEN_W || y < 0 || y >= SCREEN_H) return;

    int word_addr  = (y * WORDS_PER_LINE) + (x / 8);
    int nibble_pos = 7 - (x % 8);
    int shift      = nibble_pos * 4;

    unsigned int old  = read_word(word_addr);
    unsigned int mask = ~(0xFu << shift);
    write_word(word_addr, (old & mask) | ((unsigned int)(color & 0xF) << shift));
}

/* Replica el nibble de color en los 8 nibbles de una palabra de 32 bits */
unsigned int color_to_word(unsigned char color) {
    unsigned int c = color & 0xF;
    return c | (c << 4) | (c << 8) | (c << 12) | 
           (c << 16) | (c << 20) | (c << 24) | (c << 28);
}

/* Rellena toda la pantalla con el color dado */
void clear_screen(unsigned char color) {
    unsigned int pattern = color_to_word(color);
    for (int i = 0; i < TOTAL_WORDS; i++) {
        write_word(i, pattern);
    }
}

/*
 * Dibuja un rectangulo solido de w x h pixeles en (x, y).
 * x y w deben ser multiplos de 8 para que start_word y words_per_row
 * sean enteros exactos y no haya mezcla de nibbles en los bordes.
 */
void draw_rect(int x, int y, int w, int h, unsigned char color) {
    unsigned int pattern = color_to_word(color);
    
    int start_word    = (y * WORDS_PER_LINE) + (x / 8);
    int words_per_row = w / 8;
    
    for (int row = 0; row < h; row++) {
        int word_addr = start_word + (row * WORDS_PER_LINE);
        for (int col = 0; col < words_per_row; col++) {
            write_word(word_addr + col, pattern);
        }
    }
}

/*
 * Dibuja una linea horizontal de longitud w a partir de (x, y).
 * Maneja pixeles no alineados al inicio y al final con draw_pixel,
 * y escribe palabras completas (8 pixeles) en la parte central.
 */
void draw_hline(int x, int y, int w, unsigned char color) {
    if (y < 0 || y >= SCREEN_H) return;
    if (x < 0) { w += x; x = 0; }
    if (x + w > SCREEN_W) w = SCREEN_W - x;
    if (w <= 0) return;

    unsigned int full_word = color_to_word(color);
    int x_end = x + w;

    /* Pixeles iniciales hasta el siguiente limite de palabra */
    while (x < x_end && (x % 8) != 0) {
        draw_pixel(x, y, color);
        x++;
    }

    /* Palabras completas: 8 pixeles por escritura */
    while (x + 8 <= x_end) {
        write_word((y * WORDS_PER_LINE) + (x / 8), full_word);
        x += 8;
    }

    /* Pixeles finales tras el ultimo limite de palabra */
    while (x < x_end) {
        draw_pixel(x, y, color);
        x++;
    }
}

/*
 * Dibuja una linea vertical de altura h en la columna x.
 * Se alinea x al multiplo de 8 inferior y usa draw_rect de 8px de ancho
 * porque write_word es la operacion mas confiable en este IP VGA.
 */
void draw_vline(int x, int y, int h, unsigned char color) {
    if (x < 0 || x >= SCREEN_W) return;
    if (y < 0) { h += y; y = 0; }
    if (y + h > SCREEN_H) h = SCREEN_H - y;
    if (h <= 0) return;

    int x_aligned = x & ~7;
    draw_rect(x_aligned, y, 8, h, color);
}

/* Dibuja un borde de 1 pixel en los cuatro lados de la pantalla */
void draw_border(unsigned char color) {
    draw_hline(0,            0,            SCREEN_W, color);  // top
    draw_hline(0,            SCREEN_H - 1, SCREEN_W, color);  // bottom
    draw_vline(0,            0,            SCREEN_H, color);  // left
    draw_vline(SCREEN_W - 1, 0,            SCREEN_H, color);  // right
}

/* Delay bloqueante aproximado en milisegundos */
void delay_ms(int ms) {
    for (volatile int i = 0; i < ms * 100000; i++);
}

/*
 * Copia una imagen 480x480 a 4bpp centrada horizontalmente en pantalla
 * (80px de margen negro a cada lado en una pantalla de 640px de ancho).
 * Los bytes de imagen se leen en grupos de 4 (un word) y se invierten
 * para que el primer byte del archivo quede en los nibbles mas altos.
 */
void draw_image_centered(unsigned char *image_data) {
    int start_x = (SCREEN_W - 480) / 2;  /* (640 - 480) / 2 = 80 */
    int start_y = (SCREEN_H - 480) / 2;  /* (480 - 480) / 2 = 0  */

    /* Franjas negras laterales */
    draw_rect(0,             0, start_x, SCREEN_H, COLOR_BLACK);
    draw_rect(start_x + 480, 0, start_x, SCREEN_H, COLOR_BLACK);

    /* Copiar imagen desde DDR2 a VGA */
    uint32_t src_offset = 0;
    for (int y = 0; y < 480; y++) {
        for (int x = 0; x < 480; x += 8) {  /* 8 pixeles = 1 word */
            int word_addr = ((start_y + y) * WORDS_PER_LINE) + ((start_x + x) / 8);
            uint32_t word_val = 0;

            /* Leer 4 bytes (8 pixeles a 4bpp) e invertir orden para que
               el primer byte del archivo ocupe los nibbles mas altos */
            for (int px = 0; px < 4; px++) {
                uint32_t byte_val = image_data[src_offset++];
                word_val |= (byte_val << ((3 - px) * 8));
            }

            write_word(word_addr, word_val);
        }
    }
}
