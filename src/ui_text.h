#ifndef UI_TEXT_H
#define UI_TEXT_H

/*
 * ============================================================================
 * ui_text.h - Centered Text UI Module
 * ============================================================================
 * Generic text rendering with horizontal centering on 640px display.
 * Dependencies: font.h, vga.h
 * ============================================================================
 */

/*
 * ui_draw_text_centered(y, text, color)
 * Renders string centered horizontally at Y coordinate.
 *
 * Parameters:
 *   y:     Y coordinate in pixels (0-479)
 *   text:  Null-terminated string pointer
 *   color: 4-bit color index (0x0-0xF)
 *
 * Returns: void
 * Centering formula: x = (640 - strlen(text)*8) / 2
 * Minimum x: 0 (no overflow to left)
 */
void ui_draw_text_centered(int y, const char *text, unsigned char color);

#endif
