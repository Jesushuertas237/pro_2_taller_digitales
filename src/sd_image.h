#ifndef SD_IMAGE_H
#define SD_IMAGE_H

#include "xil_io.h"

/* Direccion base en DDR2 donde se carga la imagen; tamaño para 480x480 a 4bpp */
#define DDR2_IMAGE_BASE 0x80100000
#define IMAGE_SIZE      (480 * 480 / 2)

/* Inicializa el controlador SD. Debe llamarse una vez antes de sd_image_load().
   Retorna 0 en exito, -1 en error. */
int sd_image_init(void);

/* Carga un archivo BIN del directorio raiz de la SD a DDR2_IMAGE_BASE.
   Retorna 0 en exito, -1 en error. */
int sd_image_load(const char *filename);

/* Retorna puntero al buffer de imagen en DDR2. Valido tras sd_image_load() exitoso. */
unsigned char* sd_image_get_buffer(void);

#endif
