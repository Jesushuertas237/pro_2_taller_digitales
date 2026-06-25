/*
 * ============================================================================
 * font.h - Font Rendering Module
 * ============================================================================
 * Character and string rendering on VGA display (640x480, 8x8 font).
 * Dependencies: vga.h
 * ============================================================================
 */

#ifndef FONT_H
#define FONT_H

#include "xil_types.h"

#define CHAR_WIDTH  8
#define CHAR_HEIGHT 8

/*
 * draw_char(x, y, c, color)
 * Renders single character at (x,y) in VGA buffer.
 *
 * Parameters:
 *   x:     X coordinate in pixels (0-639)
 *   y:     Y coordinate in pixels (0-479)
 *   c:     Character (A-Z, 0-9, space, punctuation)
 *   color: 4-bit color index (0x0-0xF)
 *
 * Returns: void
 * Character width: 8 pixels, height: 8 pixels
 */
void draw_char(int x, int y, char c, unsigned char color);

/*
 * draw_string(x, y, str, color)
 * Renders null-terminated string starting at (x,y).
 *
 * Parameters:
 *   x:     X coordinate in pixels
 *   y:     Y coordinate in pixels
 *   str:   Null-terminated string pointer
 *   color: 4-bit color index (0x0-0xF)
 *
 * Returns: void
 * Character spacing: 8 pixels horizontal
 */
void draw_string(int x, int y, const char *str, unsigned char color);

/*
 * draw_char_big(x, y, c, color, vscale)
 * Renders magnified character using draw_rect() for each pixel.
 * Avoids nibble-mixing artifacts in VGA IP by writing 8px-wide blocks.
 *
 * Parameters:
 *   x:       X coordinate in pixels
 *   y:       Y coordinate in pixels
 *   c:       Character
 *   color:   4-bit color index
 *   vscale:  Vertical scale factor
 *
 * Returns: void
 * Magnified size: 64px wide x (8*vscale)px high
 */
void draw_char_big(int x, int y, char c, unsigned char color, int vscale);

/*
 * draw_string_big(x, y, str, color, vscale)
 * Renders magnified string with vertical scaling.
 *
 * Parameters:
 *   x:       X coordinate in pixels
 *   y:       Y coordinate in pixels
 *   str:     Null-terminated string pointer
 *   color:   4-bit color index
 *   vscale:  Vertical scale factor
 *
 * Returns: void
 */
void draw_string_big(int x, int y, const char *str, unsigned char color, int vscale);

/*
 * draw_char_rot(x, y, c, color, rot)
 * Renders character with circular nibble offset.
 *
 * Parameters:
 *   x:     X coordinate in pixels
 *   y:     Y coordinate in pixels
 *   c:     Character
 *   color: 4-bit color index
 *   rot:   Nibble offset (0-7, circular)
 *
 * Returns: void
 * Precondition: rot = 0 is standard behavior
 */
void draw_char_rot(int x, int y, char c, unsigned char color, int rot);

/*
 * draw_char_xform(x, y, c, color, rot, mirror)
 * Renders character with nibble offset and horizontal flip.
 *
 * Parameters:
 *   x:      X coordinate in pixels
 *   y:      Y coordinate in pixels
 *   c:      Character
 *   color:  4-bit color index
 *   rot:    Nibble offset (0-7)
 *   mirror: Horizontal flip (0=no, 1=yes)
 *
 * Returns: void
 */
void draw_char_xform(int x, int y, char c, unsigned char color, int rot, int mirror);

#endif