#include "system.h"
#include "scheduler.h"
#include "asl.h"
#include "syscall.h"
#include "handler.h"
#include "kprintf.h"
#include "interrupt.h"

/*SysCall 1.
Quando invocata restituisce il tempo di esecuzione del processo chiamante
suddiviso in user_time kernel_time e global_time, il tempo totale trasccorso
dalla prima attivazione del processo. */
void syscallGetCPUTime(unsigned int* user, unsigned int* kernel, unsigned int *wallclock)
{
    pcb_t* current_proc = getCurrentProcess();
    *user = current_proc->user_time;
    *kernel = current_proc->kernel_time;
    *wallclock = (getTime() - current_proc->begin_timestamp);
}

/* SysCall 2.
Crea un nuovo processo come figlio del chiamante, se la syscall ha successo
cpid (se non NULL) continene l'indirizzo del pcb_t del processo figlio creato */
int syscallCreateProcess(state_t* statep, int priority, void **cpid)
{
    pcb_t* child_proc = allocPcb();
    
    if(child_proc)
    {
        pcb_t* current_proc = getCurrentProcess();
        insertChild(current_proc, child_proc);
        if(cpid)
        {
            *cpid = child_proc;
        }
        
        state_t* status = &child_proc->p_s;
        
        STATUS(status) = STATUS(statep);
        PC(status) = PC(statep);
        SP(status) = SP(statep);
        
        addProcess(child_proc, priority);
        return 0;
    }
    
    return -1;
}

/*SysCall 3.
Termina il processo identificato dal parametro pid e l'albero di processi radicato in esso,
se non esiste ritorna -1. Se pid è NULL termina il processo corrente*/
int syscallTerminateProcess(void* pid)
{
    
    pcb_t* p = (pcb_t*)pid;
    if(!p)
    {
        terminateCurrentProcess();
        return 0;
    }
    else
    {
        return terminateProcess(p);
    }
}

/*SysCall 4.
Operazione di rilascio sul semaforo identificato dal valore di semaddr */
void syscallVerhogen(int* semaddr)
{
    pcb_t* p = removeBlocked(semaddr);
    if(!p)
    {
        *semaddr += 1;
    }
    else
    {
        resumeProcess(p);
    }
}

/*SysCall 5.
Operazione di richiesta di un semaforo */
void syscallPasseren(int* semaddr)
{
    if(*semaddr == 0)
    {
        pcb_t* p = suspendCurrentProcess();
        if(insertBlocked(semaddr, p))
        {
            kprintf("Unable to allocate semaphore!\n");
            PANIC();
        }
    }
    else
    {
        *semaddr -= 1;
    }
}

//Ritorna 1 se dev e' un indirizzo valido per un device e 0 altrimenti
static int IsValidDeviceAddress(void* dev)
{
    unsigned int start = DEV_REG_ADDR(DEV_IL_START, 0);
    unsigned int end = DEV_REG_END;
    
    unsigned int address = (unsigned int)dev;
    
    //Controlla che sia contenuto nell'area dei device
    if(start <= address && address < end)
    {
        //Controlla che sia allineato a un device in modo corretto
        address = address - start;
        if((address & 0xF) == 0)
        {
            return 1;
        }
    }
    
    return 0;
}

/*SysCall 6.
Attiva un'operazione di input/output. L'operazione è bloccante */
void syscallDo_IO(unsigned int command, unsigned int *reg, int subdevice)
{
    //Viene definito un device generico
    devreg_t* dev = (devreg_t*)reg;
    //Il processo corrente viene rimosso dalla ready_queue
    pcb_t* process = suspendCurrentProcess();
    
    //Validazione device
    if(!IsValidDeviceAddress(dev))
    {
        //In caso di errore terminiamo il processo corrente
        terminateCurrentProcess();
    }
    else
    {
        //Utilizziamo l'indirizzo del device come chiave del semaforo su cui il processo
        //rimane in attesa del completamento dell'operazione, nel caso di terminali
        //utilizziamo due indirizzi distinti per operazioni di trasmissione e ricezione
        int *key;
        unsigned int* command_register;
        
        if((unsigned int)reg < DEV_REG_ADDR(IL_TERMINAL,0))
        {
            command_register = &dev->dtp.command;
            key = (int*)dev;
        }
        else
        {
            //Identificazione del subdevice nel caso del terminale
            if(subdevice)
            {
                command_register = &dev->term.recv_command;
                key = (int*)&dev->term.recv_status;
            }
            else
            {
                command_register = &dev->term.transm_command;
                key = (int*)&dev->term.transm_status;
            }
        }
        
        //Se il device ha gia' un processo in attesa di completamento  mettiamo il processo
        //sulla coda per l'utilizzo del device.
        if(getSemd(key))
        {
            int* wait_key = getWaitKeyFromDeviceKey(key);
            insertBlocked(wait_key, process);
        }
        //Altrimenti eseguiamo il comando e inseriamo il processo corrente in coda 
        //per il completamento dell'operazione
        else
        {
            *command_register = command;
            insertBlocked(key, process);
        }
    }
}

/*SysCall 7.
Registra l'handler di livello superiore da attivare in caso di trad si Sys/BP, TLB o Trap. */
int syscallSpecPassup(int type, state_t* old, state_t* new)
{
    //Verifica che il type sia nel range consentito
    if(0 <= type && type < SPECPASSUP_NUM_TYPES)
    {
        pcb_t* current = getCurrentProcess();
        handler_areas* current_areas = &current->specpassup_areas[type];
        
        //Verifica che i campi relativiall'handler non siano già stati impostati
        if(!current_areas->old_area && !current_areas->new_area)
        {
            current_areas->old_area = old;
            current_areas->new_area = new;
            
            return 0;
        }
    }
        
    //In caso di errore terminiamo il processo corrente
    terminateCurrentProcess();
    
    return -1;
}
/*SysCall 8.
Ritorna in pid e ppid (se non sono NULL) l'indirizzo del pcb del processo corrente e
del padre rispettivamente */
void syscallGetPidPPid(void** pid, void** ppid)
{
    pcb_t* currentProcess = getCurrentProcess();
    pcb_t* parentProcess = currentProcess->p_parent;
    if(pid)
    {
        *pid =  currentProcess;
    }
    if(ppid)
    {
        *ppid = parentProcess;
    }
}
