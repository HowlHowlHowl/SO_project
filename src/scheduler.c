#include "scheduler.h"
#include "system.h"
#include "kprintf.h"
#include "utils.h"

//Macro per la gestione dei registri dello stato in modo analogo per entrambe le architetture

#ifdef TARGET_UMPS
#define STATE_EXCCODE(s) CAUSE_GET_EXCCODE((s)->cause)
#define STATE_SYSCALL_NUMBER(s) (s)->reg_a0
#define STATE_SYSCALL_P1(s) (s)->reg_a1
#define STATE_SYSCALL_P2(s) (s)->reg_a2
#define STATE_SYSCALL_P3(s) (s)->reg_a3
#define STATE_SYSCALL_RETURN(s) (s)->reg_v0
#define STATE_CAUSE(s) (s)->cause
#define getTODLO() (*(unsigned int *)BUS_TODLOW)
#define BUS_TODLOW 0x1000001c

#define CAUSE_IP_GET(cause, n) ((cause) & (CAUSE_IP(n)))
#endif
//Definita in modo analogo a uarm
#ifdef TARGET_UARM
#define STATE_EXCCODE(s) CAUSE_EXCCODE_GET((s)->CP15_Cause)
#define STATE_SYSCALL_NUMBER(s) (s)->a1
#define STATE_SYSCALL_P1(s) (s)->a2
#define STATE_SYSCALL_P2(s) (s)->a3
#define STATE_SYSCALL_P3(s) (s)->a4
#define STATE_SYSCALL_RETURN(s) (s)->a1
#define STATE_CAUSE(s) (s)->CP15_Cause
#endif

#define TIME_SLICE_DURATION_MS 3
//time-stamp del tempo in cui viene passato il controllo al processo
static unsigned int current_slice_timestamp;

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
    p->begin_timestamp = getTime();
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


//Termina il processo corrente e l'albero radicato in esso
void terminateCurrentProcess(void)
{
    removePcbRecursively(current_process);
}

//Termina il processo passato come parametro e l'albero radicato in esso
void terminateProcess(pcb_t* p)
{
    removePcbRecursively(p);
}

// Inserisce p nella coda, la priorita' di p e' gia' stata impostata in precedenza
void resumeProcess(pcb_t* p)
{
    insertProcQ(&ready_queue, p);
}

//Rimuove dalla ready queue e ritorna il processo corrente
pcb_t* suspendCurrentProcess(void)
{
    return outProcQ(&ready_queue,current_process);
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

