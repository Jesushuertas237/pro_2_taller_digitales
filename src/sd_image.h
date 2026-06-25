#ifndef SD_IMAGE_H
#define SD_IMAGE_H

#include "xil_io.h"

/*
 * ============================================================================
 * sd_image.h - SD Card Image Loader
 * ============================================================================
 * Loads 480x480 image files from SD card to DDR2 buffer.
 * Image format: 4 bits per pixel (2 pixels per byte)
 * ============================================================================
 */

#ifndef SD_IMAGE_H
#define SD_IMAGE_H

#include "xil_io.h"

#define DDR2_IMAGE_BASE  0x80100000
#define IMAGE_SIZE       (480 * 480 / 2)

/*
 * sd_image_init(void)
 * Initializes SD card controller.
 *
 * Returns: 0 on success, <0 on initialization error
 * Precondition: SD card must be present and formatted
 * Must be called once before sd_image_load()
 */
int sd_image_init(void);

/*
 * sd_image_load(filename)
 * Loads image file from SD card to DDR2.
 *
 * Parameters:
 *   filename: File name in SD root (e.g., "MENU.BIN", "TWO.BIN")
 *
 * Returns: 0 on success, <0 on read error
 * Precondition: sd_image_init() must have been called
 * File must be exactly 460800 bytes (480x480x4bits)
 */
int sd_image_load(const char *filename);

/*
 * sd_image_get_buffer(void)
 * Returns pointer to loaded image buffer in DDR2.
 *
 * Returns: unsigned char* pointing to DDR2_IMAGE_BASE
 * Postcondition: Buffer valid only after successful sd_image_load()
 */
unsigned char* sd_image_get_buffer(void);

#endif

#endif
