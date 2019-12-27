#include "system.h"
#include "term_read.h"

#define RS_READY 1
#define RS_BUSY 3
#define RS_RECEIVED 5

#define CMD_ACK 1
#define CMD_RECEIVECHAR 2

#define RECV_STATUS_MASK 0xFF
#define RECV_CHAR_OFFSET 8

static termreg_t *term0_reg = (termreg_t *) DEV_REG_ADDR(IL_TERMINAL, 0);

/* Returns the next available char, or zero on error */
static char term_recv_char()
{
    int status;
    
    status = term0_reg->recv_status & RECV_STATUS_MASK;
    if(!(status == RS_RECEIVED || status == RS_READY))
    {
        return 0;
    }
    
    term0_reg->recv_command = CMD_RECEIVECHAR;
    
    while(((status = term0_reg->recv_status) & RECV_STATUS_MASK) == RS_BUSY)
        ;
    
    char c = 0;
    if((status & RECV_STATUS_MASK) == RS_RECEIVED)
    {
        c = term0_reg->recv_status >> RECV_CHAR_OFFSET;        
    }

    term0_reg->recv_command = CMD_ACK;
    return c;
}

void term_read_line(char* data, int max_size)
{
    int i = 0;
    
    /* Leave 1 char for null termination */
    while(i < max_size - 1)
    {
        char c = term_recv_char();
        if(c == 0)
        {
            break;
        }
        
        data[i] = c;
        i++;
        
        if(c == '\n')
        {
            break;
        }
    }
    
    data[i] = 0;
}

