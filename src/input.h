/*
 * ============================================================================
 * input.h - GPIO Input Module (Nexys A7)
 * ============================================================================
 * Button and switch input with software debounce.
 * Hardware: Xilinx GPIO IP at specified base addresses
 * ============================================================================
 */

#ifndef INPUT_H
#define INPUT_H

#define GPIO_BUTTONS_BASE   0x40010000
#define GPIO_SWITCHES_BASE  0x40020000
#define GPIO_DATA_REG       0x00
#define GPIO_TRI_REG        0x04

/*
 * input_init(void)
 * Initializes GPIO ports for input operation.
 *
 * Configures both button and switch ports by writing 0xFFFFFFFF to TRI_REG.
 * Must be called once before any input operations.
 *
 * Returns: void
 */
void input_init(void);

/*
 * input_read_buttons(void)
 * Reads debounced button state.
 *
 * Returns: 4-bit mask (bit 0=BTNC, bit 1=BTNU, bit 2=BTNL, bit 3=BTNR)
 * Non-zero only when state is stable (≥2 identical reads)
 * Software debounce with state machine
 */
unsigned int input_read_buttons(void);

/*
 * input_read_switches(void)
 * Reads debounced switch state.
 *
 * Returns: 16-bit mask (bit N = SW_N state)
 * Non-zero only when state is stable
 * Software debounce with state machine
 */
unsigned int input_read_switches(void);

#endif
