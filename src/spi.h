#ifndef SPI_H
#define SPI_H

#include "xil_io.h"

/* Registros del periferico GPIO_3 mapeado al PMOD JA */
#define SPI_DATA 0x40030000
#define SPI_TRI  0x40030004

/* Posiciones de bit en GPIO_3 — pines fisicos del PMOD JA */
#define PIN_SCK  (1u << 0)   /* JA1 - C17 */
#define PIN_MOSI (1u << 1)   /* JA2 - D18 */
#define PIN_MISO (1u << 2)   /* JA3 - E18 */
#define PIN_SS   (1u << 3)   /* JA4 - G17 */

/* Registro TRI: 0 = salida, 1 = entrada */
#define TRI_MASTER (PIN_MISO)                     /* maestro conduce SCK, MOSI, SS; lee MISO */
#define TRI_SLAVE  (PIN_SCK | PIN_MOSI | PIN_SS)  /* esclavo conduce MISO; lee SCK, MOSI, SS */

/*
 * Conexion fisica entre dos boards via PMOD JA:
 *   JA1 (SCK)  maestro -> esclavo
 *   JA2 (MOSI) maestro -> esclavo JA3
 *   JA3 (MISO) maestro <- esclavo JA2
 *   JA4 (SS)   maestro -> esclavo
 */

void spi_init_master(void);
void spi_init_slave(void);

/* Transfiere un byte como maestro; retorna el byte recibido del esclavo */
unsigned char spi_master_transfer(unsigned char tx);

/* Transfiere un byte como esclavo; escribe el byte recibido en *rx_out.
   Retorna 0 en exito, -1 si hay timeout esperando flanco de SCK. */
int spi_slave_transfer(unsigned char tx, unsigned char *rx_out);

#endif
