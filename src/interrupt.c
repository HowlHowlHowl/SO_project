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
void wakeUpProcess(int* key, unsigned int status, unsigned int kernel_time_begin)
{
    pcb_t* p = removeBlocked(key);
    
    if(!p)
    {
        kprintf("Error: Interrupt on line with no process waiting\n");
        return;
    }
    
    //Inserisce lo status come valore di ritorno per la syscall Do_IO
    state_t* state = &p->p_s;
    STATE_SYSCALL_RETURN(state) = status;
    
    //Incrementa del tempo necessario a gestire questo device il kernel time del processo
    p->kernel_time += getTime() - kernel_time_begin;

    //Reinserisce il processo nella ready queue
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
                    //Salviamo il timestamp del momento in cui abbiamo iniziato a gestire l'interrupt
                    unsigned int kernel_time_begin = getTime();
                    unsigned int status;
                    
                    devreg_t* dev = (devreg_t*)DEV_REG_ADDR(line, device);
                    
                    //Gestisci i terminali separatamente dato che sono composti da 2 subdevice
                    if(line == IL_TERMINAL)
                    {
                        status = dev->term.recv_status;
                        if((status & TERM_STATUS_MASK) == DEV_TRCV_S_CHARRECV)
                        {
                            dev->term.recv_command = CMD_ACK;
                            wakeUpProcess((int*)&dev->term.recv_status, status, kernel_time_begin);
                            result = 1;
                        }
                        
                        status = dev->term.transm_status;
                        if((status & TERM_STATUS_MASK) == DEV_TTRS_S_CHARTRSM)
                        {
                            dev->term.transm_command = CMD_ACK;
                            wakeUpProcess((int*)&dev->term.transm_status, status, kernel_time_begin);
                            result = 1;
                        }
                    }
                    else
                    {
                        status = dev->dtp.status;
                        dev->dtp.command = CMD_ACK;
                        wakeUpProcess((int*)dev, status, kernel_time_begin);
                        result = 1;
                    }
                }
            }
        }
    }
    
    return result;
}
