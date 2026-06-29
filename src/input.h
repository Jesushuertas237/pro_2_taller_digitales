/*
 * input.h - Lectura de botones y switches del Nexys A7 con debounce por software.
 * Hardware: Xilinx GPIO IP
 */
#ifndef INPUT_H
#define INPUT_H

/* Direcciones base de los perifericos GPIO */
#define GPIO_BUTTONS_BASE  0x40010000
#define GPIO_SWITCHES_BASE 0x40020000

/* Offsets de registros GPIO */
#define GPIO_DATA_REG 0x00
#define GPIO_TRI_REG  0x04

/* Configura los puertos de botones y switches como entradas.
   Debe llamarse una vez antes de cualquier lectura. */
void input_init(void);

/* Retorna mascara de 4 bits con flancos de subida detectados en botones.
   Bit 0=BTNC, 1=BTNU, 2=BTNL, 3=BTNR. */
unsigned int input_read_buttons(void);

/* Retorna mascara de 16 bits con flancos de subida detectados en switches.
   Bit N corresponde a SW_N. */
unsigned int input_read_switches(void);

#endif
