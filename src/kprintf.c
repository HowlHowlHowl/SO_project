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
  %b stampa un intero unsigned in 32 cifre binarie, con uno spazio ogni 8 cifre
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
                } break;
                
                case 'b':
                {
                    unsigned int n = va_arg(va, unsigned int);
                    for(int i = 31; i >= 0; i--)
                    {
                        unsigned int v;
                        
                        //shift di 0 su uarm ritorna sempre 0
                        if(i == 0) 
                        {
                            v = n;
                        }
                        else
                        {
                            v = n >> i;
                        }
                        
                        char bit = v & 1;
                        term_putchar(term, '0' + bit);
                        
                        //se e' un multiplo di 8 stampiamo uno spazio
                        if(i && (i & 7) == 0)
                        {
                            term_putchar(term, ' ');
                        }
                    }
                } break;
            }
        }
        else
        {
            term_putchar(term, c);
        }
    }
    
    va_end(va);
}
