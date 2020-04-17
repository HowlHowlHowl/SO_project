#include "kprintf.h"
#include "system.h"
#include "scheduler.h"
#include "const_bikaya.h"
#include "term_print.h"
#include "asl.h"

//TODO: sposta sta roba nel file handler.c
#ifdef TARGET_UMPS
#define STATE_EXCCODE(s) CAUSE_GET_EXCCODE((s)->cause)
#define STATE_SYSCALL_NUMBER(s) (s)->reg_a0
#define STATE_SYSCALL_RETURN(s) (s)->reg_v0
#define STATE_CAUSE(s) (s)->cause
//Definita in modo analogo a uarm
#define CAUSE_IP_GET(cause, n) ((cause) & (CAUSE_IP(n)))
#endif

#ifdef TARGET_UARM
#define STATE_EXCCODE(s) CAUSE_EXCCODE_GET((s)->CP15_Cause)
#define STATE_SYSCALL_NUMBER(s) (s)->a1
#define STATE_SYSCALL_RETURN(s) (s)->a1
#define STATE_CAUSE(s) (s)->CP15_Cause
#endif

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
        //la chiamata schedule() non ritorna
        updateCurrentProcess(old_state);
        schedule();
    }
    else
    {
        //Altrimenti riprendi l'esecuzione del processo precedente
        LDST(old_state);
    }
}

