#include "scheduler.h"
#include "system.h"
#include "kprintf.h"
#include "utils.h"

#define TIME_SLICE_DURATION_MS 3

static struct list_head ready_queue;
static pcb_t* current_process;
static pcb_t* idle_process;

// Inizializza la coda dei processi
void initScheduler(void)
{
    mkEmptyProcQ(&ready_queue);
}

// Aggiunge un processo alla coda con la priorita' specificata
void addProcess(pcb_t* p, int priority)
{
    p->priority = priority;
    p->original_priority = priority;
    insertProcQ(&ready_queue, p);
}

// Rimuove il processo p e l'albero di processi radicato in esso in modo ricorsivo
static void removePcbRecursively(pcb_t* p)
{
    pcb_t* removed = outProcQ(&ready_queue, p);
    if(removed)
    {
        freePcb(removed);
    }
    
    pcb_t* pos;
	list_for_each_entry(pos, &p->p_child, p_sib)
	{
		removePcbRecursively(pos);
	}
}

//Imposta p come processo idle, da mandare in esecuzione quando la coda dei processi e' vuota
void setIdleProcess(pcb_t* p)
{
    idle_process = p;
}

//Aggiorna lo stato del processo corrente allo stato passato come parametro
void updateCurrentProcess(state_t* state)
{
    copy_memory(&current_process->p_s, state, sizeof(state_t));
}

//Termina il processo corrente e l'labero radicato in esso
void terminateCurrentProcess(void)
{
    removePcbRecursively(current_process);
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
    resetIntervalTimer();
    current_process = p;
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
