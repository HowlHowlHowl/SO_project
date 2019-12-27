#include "system.h"

#include "term_read.h"
#include "printer.h"

int main(void)
{
    char string[256];
    term_read_line(string, sizeof(string)); 
    printer_puts(string);
    
    while (1) 
        WAIT();
    return 0;
}

