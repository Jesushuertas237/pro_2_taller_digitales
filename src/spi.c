#include "spi.h"

static void spi_delay(volatile int n) { while (n--); }

/* Configura el GPIO como maestro SPI y deja SS inactivo (alto) */
void spi_init_master(void) {
    Xil_Out32(SPI_TRI, TRI_MASTER);
    Xil_Out32(SPI_DATA, PIN_SS);
}

/* Configura el GPIO como esclavo SPI */
void spi_init_slave(void) {
    Xil_Out32(SPI_TRI, TRI_SLAVE);
    Xil_Out32(SPI_DATA, 0u);
}

/*
 * Transfiere un byte como maestro SPI (modo 0: CPOL=0, CPHA=0).
 * Genera el reloj por software: pone MOSI, sube SCK, muestrea MISO, baja SCK.
 * Transmite y recibe MSB primero.
 */
unsigned char spi_master_transfer(unsigned char tx) {
    unsigned char rx = 0;
    unsigned int mosi_bit;
    int bit;

    for (bit = 7; bit >= 0; bit--) {
        mosi_bit = (tx & (1u << bit)) ? PIN_MOSI : 0u;
        Xil_Out32(SPI_DATA, mosi_bit);           /* MOSI valido, SCK bajo */
        spi_delay(50);
        Xil_Out32(SPI_DATA, mosi_bit | PIN_SCK); /* flanco de subida SCK */
        spi_delay(50);
        if (Xil_In32(SPI_DATA) & PIN_MISO)
            rx |= (1u << bit);                   /* muestrear MISO en flanco alto */
        Xil_Out32(SPI_DATA, mosi_bit);           /* flanco de bajada SCK */
        spi_delay(50);
    }
    return rx;
}

/*
 * Recibe un byte como esclavo SPI y transmite tx en la misma transferencia.
 * Espera el flanco de subida de SCK para muestrear MOSI y el de bajada
 * antes de pasar al siguiente bit. Retorna 0 en exito, -1 si se agota
 * el timeout en cualquier flanco (maestro ausente o SS no activo).
 */
int spi_slave_transfer(unsigned char tx, unsigned char *rx_out) {
    unsigned char rx = 0;
    int bit, t;

    for (bit = 7; bit >= 0; bit--) {
        /* Poner MISO antes del flanco de subida para que el maestro lo muestree */
        Xil_Out32(SPI_DATA, (tx & (1u << bit)) ? PIN_MISO : 0u);

        t = 500000;
        while (!(Xil_In32(SPI_DATA) & PIN_SCK) && --t);
        if (!t) return -1;   /* timeout esperando flanco de subida */

        if (Xil_In32(SPI_DATA) & PIN_MOSI)
            rx |= (1u << bit);

        t = 500000;
        while ((Xil_In32(SPI_DATA) & PIN_SCK) && --t);
        if (!t) return -1;   /* timeout esperando flanco de bajada */
    }

    *rx_out = rx;
    return 0;
}
