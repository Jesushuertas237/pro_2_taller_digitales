/*
 * ============================================================================
 * vga.h - VGA Driver for Nexys A7
 * ============================================================================
 * 640x480 pixels, 4 bits per pixel (16-color palette)
 * Hardware: Xilinx VGA IP at base address 0xC0000000
 * Buffer: DDR2, word-addressed (32 bits = 8 pixels)
 * ============================================================================
 */

#ifndef VGA_H
#define VGA_H

#include "xil_io.h"

#define VGA_BASE        0xC0000000
#define SCREEN_W        640
#define SCREEN_H        480
#define WORDS_PER_LINE  80
#define TOTAL_WORDS     38400

/* 4-bit Color Palette */
#define COLOR_BLACK        0x0
#define COLOR_BLUE_DARK    0x1
#define COLOR_GREEN_DARK   0x2
#define COLOR_CYAN_DARK    0x3
#define COLOR_RED_DARK     0x4
#define COLOR_MAGENTA_DARK 0x5
#define COLOR_ORANGE       0x6
#define COLOR_GRAY_LIGHT   0x7
#define COLOR_GRAY_DARK    0x8
#define COLOR_BLUE_LIGHT   0x9
#define COLOR_GREEN_LIGHT  0xA
#define COLOR_CYAN_LIGHT   0xB
#define COLOR_RED_LIGHT    0xC
#define COLOR_MAGENTA_LIGHT 0xD
#define COLOR_YELLOW       0xE
#define COLOR_WHITE        0xF

/*
 * write_word(word_addr, value)
 * Writes 32-bit word (8 pixels) to VGA buffer.
 *
 * Parameters:
 *   word_addr: Word address (0 to TOTAL_WORDS-1)
 *   value:     32-bit value (4 nibbles)
 *
 * Returns: void
 * Precondition: word_addr must be valid
 */
void write_word(int word_addr, unsigned int value);

/*
 * read_word(word_addr)
 * Reads 32-bit word from VGA buffer.
 *
 * Parameters:
 *   word_addr: Word address (0 to TOTAL_WORDS-1)
 *
 * Returns: 32-bit value read from VGA buffer
 * Used for read-modify-write operations
 */
unsigned int read_word(int word_addr);

/*
 * color_to_word(color)
 * Converts color index to word (8 pixels of same color).
 *
 * Parameters:
 *   color: 4-bit color index (0x0-0xF)
 *
 * Returns: 32-bit word with all 8 pixels set to color
 */
unsigned int color_to_word(unsigned char color);

/*
 * clear_screen(color)
 * Fills entire VGA buffer with single color.
 *
 * Parameters:
 *   color: 4-bit color index (0x0-0xF)
 *
 * Returns: void
 * Performance: Fills 38400 words
 */
void clear_screen(unsigned char color);

/*
 * draw_pixel(x, y, color)
 * Writes single pixel at (x,y).
 *
 * Parameters:
 *   x:     X coordinate (0-639)
 *   y:     Y coordinate (0-479)
 *   color: 4-bit color index (0x0-0xF)
 *
 * Returns: void
 * Implementation: read-modify-write
 */
void draw_pixel(int x, int y, unsigned char color);

/*
 * draw_hline(x, y, w, color)
 * Draws horizontal line.
 *
 * Parameters:
 *   x:     Start X coordinate (pixels)
 *   y:     Y coordinate (pixels)
 *   w:     Width in pixels
 *   color: 4-bit color index
 *
 * Returns: void
 */
void draw_hline(int x, int y, int w, unsigned char color);

/*
 * draw_vline(x, y, h, color)
 * Draws vertical line.
 *
 * Parameters:
 *   x:     X coordinate (pixels)
 *   y:     Start Y coordinate (pixels)
 *   h:     Height in pixels
 *   color: 4-bit color index
 *
 * Returns: void
 */
void draw_vline(int x, int y, int h, unsigned char color);

/*
 * draw_rect(x, y, w, h, color)
 * Draws filled rectangle.
 *
 * Parameters:
 *   x:     Top-left X coordinate (pixels)
 *   y:     Top-left Y coordinate (pixels)
 *   w:     Width in pixels
 *   h:     Height in pixels
 *   color: 4-bit color index
 *
 * Returns: void
 */
void draw_rect(int x, int y, int w, int h, unsigned char color);

/*
 * draw_border(color)
 * Draws border around screen edges.
 *
 * Parameters:
 *   color: 4-bit color index
 *
 * Returns: void
 */
void draw_border(unsigned char color);

/*
 * delay_ms(ms)
 * Blocking delay via software loop.
 *
 * Parameters:
 *   ms: Milliseconds to delay
 *
 * Returns: void
 * Note: Duration is clock-dependent
 */
void delay_ms(int ms);

/*
 * draw_image_centered(image_data)
 * Renders 480x480 image centered on 640x480 display.
 * Fills sides with black background.
 *
 * Parameters:
 *   image_data: Pointer to image buffer in DDR2
 *
 * Returns: void
 * Precondition: image_data must be loaded via sd_image_load()
 */
void draw_image_centered(unsigned char *image_data);

#endif