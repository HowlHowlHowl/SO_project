#include "kprintf.h"
#include "system.h"
#include "scheduler.h"
#include "const_bikaya.h"
#include "term_print.h"
#include "asl.h"
#include "handler.c"

#define CMD_ACK 1

// Trova il processo in attesa sulla linea, device e subdevice specificati,
// imposta il corretto valore di ritorno dell syscall di IO e reinserisce il processo
// nella ready queue. 
void wakeUpProcess(int line, int device, int subdevice, unsigned int status)
{
    //TODO: trova la chiave in base a linea, device e subdevice
    int* key = 0;
    
    pcb_t* p = removeBlocked(key);
    if(!p)
    {
        kprintf("Error: Interrupt on line with no process waiting\n");
        return;
    }
    
    state_t* state = &p->p_s;
    STATE_SYSCALL_RETURN(state) = status;
    
    resumeProcess(p);
}

//Controlla quali interrupt line hanno un interrupt pending partendo da quella con
//priorita' maggiore, cioe' dalla linea di numero minore.
void checkDeviceInterrupts(unsigned int cause)
{
    for(int i = 0; i <= DEV_USED_INTS; i++)
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
                        if((status = rx_status(&dev->term)) == DEV_TRCV_S_CHARRECV)
                        {
                            dev->term.recv_command = CMD_ACK;
                            wakeUpProcess(line, device, 0, status);
                        }
                        if((status = tx_status(&dev->term)) == DEV_TTRS_S_CHARTRSM)
                        {
                            dev->term.transm_command = CMD_ACK;
                            wakeUpProcess(line, device, 1, status);
                        }
                    }
                    else
                    {
                        status = dev->dtp.status;
                        dev->dtp.command = CMD_ACK;
                        wakeUpProcess(line, device, 0, status);
                    }
                }
            }
        }
    }
}

//Handler per la gestione degli interrupt
void handler_int(void)
{
    state_t* old_state = (state_t*)INT_OLDAREA;
    
#ifdef TARGET_UARM
    //Come specificato al capitolo 2.5.3 del manuale di uArm il pc va decrementato di 4
    //in seguito ad un interrupt
    old_state->pc -= 4;
#endif
    
    unsigned int cause = STATE_CAUSE(old_state);
    
    checkDeviceInterrupts(cause);
    
    if(CAUSE_IP_GET(cause, IL_TIMER))
    {
        //Se il time slice e' terminato in seguito a un interrupt dell'interval timer
        //aggiorna lo stato del processo corrente e reinvoca lo scheduler,
        pcb_t* currentProc = getCurrentProcess();
        updateCurrentProcess(old_state);
        
        //Il time slice in user mode termina definitivamente e l'user time viene aggiornato
        currentProc->user_time += getTimeSliceBegin() - getTime();
        
        //la chiamata schedule() non ritorna
        schedule();
    }
    else
    {
        //Altrimenti riprendi l'esecuzione del processo precedente
        LDST(old_state);
    }
}

