#include "kprintf.h"
#include "system.h"
#include "scheduler.h"
#include "const_bikaya.h"
#include "term_print.h"
#include "asl.h"
#include "handler.h"
#include "utils.h"

#define CMD_ACK 1
#define TERM_STATUS_MASK 0xFF

// Utilizziamo il puntatore al byte sucessivo come chiave per il semaforo
// di attesa quando il device e' gia' in uso da un processo
int* getWaitKeyFromDeviceKey(int* key)
{
    return (int*)(((char*)key) + 1);
}

// Trova il processo in attesa sulla coda del device associato alla chiave specificata
// imposta il corretto valore di ritorno dell syscall di IO e reinserisce il processo
// nella ready queue, inoltre esegue il comando del prossimo processo in attesa se presente.
// Ritorna la priorita' del processo svegliato e 0 in caso di errore.
static int wakeUpProcessAndExecuteNext(int* key, unsigned int* device_command, unsigned int status, unsigned int kernel_time_begin)
{
    //Invia l'ack al device
    *device_command = CMD_ACK;
    
    //Rimuove il processo in attesa del completamento del comando
    pcb_t* p = removeBlocked(key);
    
    if(!p)
    {
        kprintf("Error: Interrupt on line with no process waiting\n");
        return 0;
    }
    
    //Inserisce lo status come valore di ritorno per la syscall Do_IO
    state_t* state = &p->p_s;
    STATE_SYSCALL_RETURN(state) = status;
    
    //Incrementa del tempo necessario a gestire questo device il kernel time del processo
    p->kernel_time += getTime() - kernel_time_begin;

    //Reinserisce il processo nella ready queue
    resumeProcess(p);
    
    //Se qualche processo era in attesa di utilizzare il device prendiamo il primo,
    //eseguiamo il suo comando e lo mettiamo in attesa sul semaforo di completamento.
    int* wait_key = getWaitKeyFromDeviceKey(key);
    pcb_t* next_p = removeBlocked(wait_key);
    if(next_p)
    {
        unsigned int command = STATE_SYSCALL_P1(&next_p->p_s);
        *device_command = command;
        insertBlocked(key, next_p);
    }
    
    //Ritorniamo la priorita' del processo che ha completato l'operazione
    return p->priority;
}

//Controlla quali interrupt line hanno un interrupt pending partendo dalla linea con
//priorita' maggiore, cioe' quella di numero minore.
//Ritorna la priorita' piu' alta tra quella dei processi svegliati, 0 se nessun processo
//e' stato svegliato in seguito a un interrupt.
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
                            int priority = wakeUpProcessAndExecuteNext((int*)&dev->term.recv_status, &dev->term.recv_command, status, kernel_time_begin);
                            result = MAX(result, priority);
                        }
                        
                        status = dev->term.transm_status;
                        if((status & TERM_STATUS_MASK) == DEV_TTRS_S_CHARTRSM)
                        {
                            int priority = wakeUpProcessAndExecuteNext((int*)&dev->term.transm_status, &dev->term.transm_command, status, kernel_time_begin);
                            result = MAX(result, priority);
                        }
                    }
                    else
                    {
                        status = dev->dtp.status;
                        int priority = wakeUpProcessAndExecuteNext((int*)dev, &dev->dtp.command, status, kernel_time_begin);
                        result = MAX(result, priority);
                    }
                }
            }
        }
    }
    
    return result;
}
