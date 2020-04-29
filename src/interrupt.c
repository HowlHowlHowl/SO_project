#include "kprintf.h"
#include "system.h"
#include "scheduler.h"
#include "const_bikaya.h"
#include "term_print.h"
#include "asl.h"
#include "handler.h"

#define CMD_ACK 1
#define TERM_STATUS_MASK 0xFF

// Trova il processo in attesa sulla linea, device e subdevice specificati,
// imposta il corretto valore di ritorno dell syscall di IO e reinserisce il processo
// nella ready queue. 
void wakeUpProcess(int* key, unsigned int status)
{
//    kprintf("Waking up process waiting of device %x", key);
    
    pcb_t* p = removeBlocked(key);
    if(!p)
    {
        kprintf("Error: Interrupt on line with no process waiting\n");
        return;
    }
    
    state_t* state = &p->p_s;
    STATE_SYSCALL_RETURN(state) = status;

//    term_putchar(TERMINAL6, status >> 8);
    
#if 0
    char buf[2];
    buf[0] = (status >> 8) & 0xFF;
    buf[1] = 0;
    kprintf("WAITIO ended with status: %x, and char %s\n", status & TERM_STATUS_MASK, buf);
#endif
    
    resumeProcess(p);
}

//Controlla quali interrupt line hanno un interrupt pending partendo da quella con
//priorita' maggiore, cioe' dalla linea di numero minore. 
//Ritorna 1 se almeno un processo e' stato risvegliato in seguito a un interrupt
int checkDeviceInterrupts(unsigned int cause)
{
    int result = 0;
    
    for(int i = 0; i < DEV_USED_INTS; i++)
    {
        int line = i + INT_LOWEST;
        if(CAUSE_IP_GET(cause, line))
        {
            //Trova quali device hanno un interrupt pending controllando la device bitmap
            unsigned int bitmap = *(unsigned int*)CDEV_BITMAP_ADDR(line);
            
            
            for(int device = 0; device < DEV_PER_INT; device++)
            {
                int pending = bitmap & 1;
                bitmap = bitmap >> 1;

                
                if(pending)
                {
                    devreg_t* dev = (devreg_t*)DEV_REG_ADDR(line, device);
                    
                    unsigned int status;
                    
                    //Gestisci i terminali separatamente dato che sono composti da 2 subdevice
                    if(line == IL_TERMINAL)
                    {
                        status = dev->term.recv_status;
                        if((status & TERM_STATUS_MASK) == DEV_TRCV_S_CHARRECV)
                        {
                            dev->term.recv_command = CMD_ACK;
                            wakeUpProcess((int*)&dev->term.recv_status, status);
                            result = 1;
                        }
                        
                        status = dev->term.transm_status;
                        if((status & TERM_STATUS_MASK) == DEV_TTRS_S_CHARTRSM)
                        {
                            dev->term.transm_command = CMD_ACK;
                            wakeUpProcess((int*)&dev->term.transm_status, status);
                            result = 1;
                        }
                    }
                    else
                    {
                        status = dev->dtp.status;
                        dev->dtp.command = CMD_ACK;
                        wakeUpProcess((int*)dev, status);
                        result = 1;
                    }
                }
            }
        }
    }
    
    return result;
}
