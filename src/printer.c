#include "system.h"
#include "printer.h"

#define PS_READY 1
#define PS_BUSY  3
#define PS_ERROR 4

#define CMD_RESET	 0
#define CMD_ACK 	 1
#define CMD_PRINTCHR 2

static dtpreg_t* print0_reg = (dtpreg_t*)DEV_REG_ADDR(IL_PRINTER, 0);

/* Prints a char, returns 0 on success */
static int printer_putc(char c)
{
    int status;
	status = print0_reg->status;
	
	if(status != PS_READY)
	{
		return -1;
	}
	print0_reg->data0 = c;
    print0_reg->command = CMD_PRINTCHR;

	while((status = print0_reg->status) == PS_BUSY)
	{
		;
	}
	
	if(status != PS_READY)
	{
		return -1;
	}

	print0_reg->command = CMD_ACK;
    
	return 0;
}

void printer_puts(const char* string)
{
    while(*string)
    {
    	if(printer_putc(*string++))
    	{
    		return;
    	}
    }
}