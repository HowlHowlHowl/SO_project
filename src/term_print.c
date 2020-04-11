#include "term_print.h"

#define ST_READY 1
#define ST_BUSY 3
#define ST_TRANSMITTED 5

#define CMD_ACK 1
#define CMD_TRANSMIT 2

#define CHAR_OFFSET 8
#define TERM_STATUS_MASK 0xFF

//Ritorna lo stato del terminale specificato
static unsigned int tx_status(termreg_t *term_reg) 
{
    return ((term_reg->transm_status) & TERM_STATUS_MASK);
}

//Stampa il carattere c sul terminale specificato
int term_putchar(termreg_t* term_reg, char c) {
    unsigned int stat;
    stat = tx_status(term_reg);
    if (stat != ST_READY && stat != ST_TRANSMITTED)
        return -1;

    term_reg->transm_command = ((c << CHAR_OFFSET) | CMD_TRANSMIT);

    while ((stat = tx_status(term_reg)) == ST_BUSY);

    term_reg->transm_command = CMD_ACK;

    if (stat != ST_TRANSMITTED)
        return -1;
    else
        return 0;
}

//Stampa la stringa zero-terminata str sul terminale specificato
int term_puts(termreg_t* term_reg, char *str) 
{
    while (*str)
    {
        if (term_putchar(term_reg, *str++)) return -1;
    }
    
    return 0;
}
