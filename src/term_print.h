#ifndef TERM_PRINT_H
#define TERM_PRINT_H

#include "system.h"

#define TERMINAL0 ((termreg_t*) DEV_REG_ADDR(IL_TERMINAL, 0))
#define TERMINAL1 ((termreg_t*) DEV_REG_ADDR(IL_TERMINAL, 1))
#define TERMINAL2 ((termreg_t*) DEV_REG_ADDR(IL_TERMINAL, 2))
#define TERMINAL3 ((termreg_t*) DEV_REG_ADDR(IL_TERMINAL, 3))
#define TERMINAL4 ((termreg_t*) DEV_REG_ADDR(IL_TERMINAL, 4))
#define TERMINAL5 ((termreg_t*) DEV_REG_ADDR(IL_TERMINAL, 5))
#define TERMINAL6 ((termreg_t*) DEV_REG_ADDR(IL_TERMINAL, 6))
#define TERMINAL7 ((termreg_t*) DEV_REG_ADDR(IL_TERMINAL, 7))

unsigned int tx_status(termreg_t *term_reg);
unsigned int rx_status(termreg_t *term_reg);
int term_putchar(termreg_t* term_reg, char c);
int term_puts(termreg_t* term_reg, char* str);

#endif

