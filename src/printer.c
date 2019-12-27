#include "system.h"
#include "printer.h"

#define ST_READY 1
#define ST_BUSY  3
#define ST_ERROR 4

#define CMD_RESET	 0
#define CMD_ACK 	 1
#define CMD_PRINTCHR 2

static dtpreg_t* print0_reg = (dtpreg_t*)DEV_REG_ADDR(IL_PRINTER, 0);

static int printer_putc(char c)
{
    unsigned int status;

	status = print0_reg->status;
	
	if(status!=ST_READY)
	{
		return -1;
	}
	print0_reg->command = (c | CMD_PRINTCHR);

	while ((status = print0_reg->status) == ST_BUSY)
	{
		;
	}
	
	if (status!=ST_READY)
	{
		return -1;
	}

	print0_reg->command = CMD_ACK;
	
	
}

void printer_puts(const char* string)
{
    while (*string)
    {
    	if(printer_putc(*string++))
    	{
    		return;
    	}
    }
}