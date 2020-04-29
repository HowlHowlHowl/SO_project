#include "scheduler.h"
#include "system.h"
#include "kprintf.h"
#include "utils.h"
#include "asl.h"

#define TIME_SLICE_DURATION_MS 3

static struct list_head ready_queue;
static pcb_t* current_process;
static pcb_t* idle_process;

//time-stamp del tempo in cui viene passato il controllo al processo
static unsigned int current_slice_timestamp;


// Inizializza la coda dei processi
void initScheduler(void)
{
    mkEmptyProcQ(&ready_queue);
}

// Aggiunge un processo alla coda con la priorita' specificata
void addProcess(pcb_t* p, int priority)
{
    kprintf("Process added %x\n", p);
    if(!p)
    {
        kprintf("Calling add process with NULL p");
        PANIC();
    }
    
    p->priority = priority;
    p->original_priority = priority;
    p->begin_timestamp = getTime();
    insertProcQ(&ready_queue, p);
}

//Imposta p come processo idle, da mandare in esecuzione quando la coda dei processi e' vuota
void setIdleProcess(pcb_t* p)
{
    idle_process = p;
}

int isIdleProcessCurrent(void)
{
    return current_process == idle_process;
}

//Aggiorna lo stato del processo corrente allo stato passato come parametro
void updateCurrentProcess(state_t* state)
{
    copy_memory(&current_process->p_s, state, sizeof(state_t));
}


// Rimuove il processo p e l'albero di processi radicato in esso in modo ricorsivo
// ritorna 0 se il processo esiste, -1 altrimenti
static int removePcbRecursively(pcb_t* p, int terminate_current)
{
    //Rimuove il processo dalla ready queue
    pcb_t* removed = outProcQ(&ready_queue, p);
    
    //Se non si trova nella ready queue lo rimuoviamo dalla coda del semaforo su cui
    //si trova in attesa
    if(!removed)
    {
        removed = outBlocked(p);
    }
    
    if(!removed)
    {
        kprintf("Attempting to remove process %x not in ready queue and not waiting on semaphore queue\n", p);
        return -1;
    }
    
    if(removed == current_process && !terminate_current)
    {
        kprintf("Attempting to terminate current_process, aes\n");
        return -1;
    }
    
    while(!emptyChild(p))
    {
        pcb_t* child = removeChild(p);
        if(removePcbRecursively(child, terminate_current) == -1)
        {
            kprintf("^ Was a child\n");
        }
    }
    outChild(p);
    
    //Se abbiamo rimosso il processo corrente lo settiamo a NULL
    if(p == current_process)
    {
        current_process = NULL;
    }
    
    kprintf("Process terminated %x\n", p);
    if(p->p_semkey) kprintf("Holy we removing process on semaphore\n");
    
//    freePcb(p);
    
    return 0;
}

//Termina il processo corrente e l'albero radicato in esso
void terminateCurrentProcess(void)
{
    removePcbRecursively(current_process, 1);
}

//Termina il processo passato come parametro e l'albero radicato in esso
//ritorna 0 se il processo esiste, -1 altrimenti
int terminateProcess(pcb_t* p)
{
    return removePcbRecursively(p, 0);
}

// Inserisce p nella coda, la priorita' di p e' gia' stata impostata in precedenza
void resumeProcess(pcb_t* p)
{
    if(!p)
    {
        kprintf("Calling resume process with NULL p");
        PANIC();
    }
    
    insertProcQ(&ready_queue, p);
}

//Rimuove dalla ready queue e ritorna il processo corrente
pcb_t* suspendCurrentProcess(void)
{
    pcb_t* p = outProcQ(&ready_queue, current_process);
    current_process = NULL;
    
    return p;
}

//Ritorna il processo corrente
pcb_t* getCurrentProcess(void)
{
    return current_process;
}

//Ritorna i primi 32 bit del Time of Day timer
unsigned int getTime(void)
{
    return getTODLO();
}

//Ritorna il timestamp a cui e' cominciato il time slice corrente
unsigned int getTimeSliceBegin(void)
{
    return current_slice_timestamp;
}

//Aggiorna il current_slice_timestamp al tempo attuale
void updateTimeSliceBegin(void) 
{
    current_slice_timestamp = getTime();    
}

//Fa ripartire l'interval timer
static void resetIntervalTimer(void)
{
    unsigned int time_scale = *(unsigned int*)BUS_REG_TIME_SCALE;
    //Durata del timer
    unsigned int timer_value = TIME_SLICE_DURATION_MS * 1000 * time_scale;
    
    *(unsigned int*)BUS_REG_TIMER = timer_value;
}

//Imposta p come processo corrente e gli passa il controllo
static void switchToProcess(pcb_t* p)
{
    if(!p)
    {
        kprintf("Calling switchToProcess with NULL p");
        PANIC();
    }
    
    resetIntervalTimer();
    current_process = p;
    updateTimeSliceBegin();
    LDST(&p->p_s);
}

//Invoca lo scheduler 
void schedule(void)
{
    //Sceglie il processo con priorita' piu' alta
    pcb_t* first = removeProcQ(&ready_queue);
    if(first)
    {
        //Imposta la priorita' al valore originale
        first->priority = first->original_priority;

        //Incrementa la priorita' di ogni altro processo
        pcb_t* it;
        list_for_each_entry(it, &ready_queue, p_next)
        {
            it->priority++;
        }

        //Reinserisce il processo scelto nella coda
        insertProcQ(&ready_queue, first);
        switchToProcess(first);
    }
    
    //Se la ready queue e' vuota passa il controllo al processo idle
    switchToProcess(idle_process);
}

