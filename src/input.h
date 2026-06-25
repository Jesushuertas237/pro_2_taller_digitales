#ifndef INPUT_H
#define INPUT_H

#define GPIO_SWITCHES_BASE 0x40000000
#define GPIO_BUTTONS_BASE  0x40010000
#define GPIO_DATA_OFFSET   0x0

typedef struct {
    unsigned int btnc;
    unsigned int btnu;
    unsigned int btnl;
    unsigned int btnr;
    unsigned int btnd;
} Buttons;

typedef struct {
    unsigned int value;
} Switches;

Switches read_switches(void);
Buttons read_buttons(void);

#endif
