#include <stdarg.h>
#include <stdbool.h>

#include "term_print.h"
#include "kprintf.h"

/*
Implementazione minimale di printf della libreria c. Stampa sul terminale 7.
Solo i seguenti format sono supportati:
  %% stampa un %
  %s stampa una string zero-terminata
  %d stampa un intero in decimale
  %u stampa un intero unsigned in decimale
  %x stampa un intero unsigned in esadecimale
*/

void kprintf(char* fmt, ...)
{
    termreg_t* term = TERMINAL7;
    
    va_list va;
    va_start(va, fmt);
    
    char hex_digits[16] = {
        '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'
    };
    
    while(*fmt)
    {
        char c = *fmt++;
        
        if(c == '%')
        {
            char f = *fmt++;
            if(f == '\0') break;
            
            switch(f)
            {
                case '%':
                {
                    term_putchar(term, '%');
                } break;
                
                case 's':
                {
                    char* s = va_arg(va, char*);
                    if(s == 0) s = "null";
                    term_puts(term, s);
                } break;
                
                case 'd':
                {
                    char buf[32];
                    char* b = buf + sizeof(buf);
                    //Null terminator
                    *--b = 0;
                    
                    int n = va_arg(va, int);
                    bool negative = false;
                    if(n < 0)
                    {
                        negative = true;
                        n = -n;
                    }
                    
                    if(n == 0)
                    {
                        *--b = '0';
                    }
                    
                    while(n > 0)
                    {
                        *--b = n % 10 + '0';
                        n /= 10;
                    }
                    
                    if(negative)
                    {
                        *--b = '-';
                    }
                    
                    term_puts(term, b);
                } break;
                
                case 'x':
                {
                    char buf[32];
                    char* b = buf + sizeof(buf);
                    //Null terminator
                    *--b = 0;
                    
                    unsigned int n = va_arg(va, unsigned int);
                    if(n == 0)
                    {
                        *--b = '0';
                    }
                    
                    while(n > 0)
                    {
                        *--b = hex_digits[n & 0xF];
                        n = n >> 4;
                    }
                    
                    term_puts(term, b);
                }break;
                
                case 'u':
                {
                    char buf[32];
                    char* b = buf + sizeof(buf);
                    //Null terminator
                    *--b = 0;
                    
                    unsigned int n = va_arg(va, unsigned int);
                    unsigned int base = 10;
                    if(n == 0)
                    {
                        *--b = '0';
                    }
                    
                    while(n > 0)
                    {
                        *--b = n % base + '0';
                        n /= base;
                    }
                    
                    term_puts(term, b);
                }
                break;
            }
        }
        else
        {
            term_putchar(term, c);
        }
    }
    
    va_end(va);
}

/* Div test 
    unsigned int a = 12;
    unsigned int b = 10;
    
    if((a / (unsigned int)10) != (a / b))
    {
        term_puts(TERMINAL0, "???");
    }
*/