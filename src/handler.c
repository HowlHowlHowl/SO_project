#include "system.h"
#include "kprintf.h"
#include "const_bikaya.h"
#include "scheduler.h"
#include "syscall.h"
#include "interrupt.h"
#include "utils.h"

// Chiama gli handler registrati dalla specpassup per il processo corrente,
// termina il processo in caso non siano stati registrati.
static void callSpecpassupHandler(state_t* state, int type)
{
    pcb_t* p = getCurrentProcess();
    handler_areas* areas = &p->specpassup_areas[type];
    if(areas->old_area && areas->new_area)
    {
        //Salva lo stato corrente nella old area e passa il controlla alla new area
        copy_memory(areas->old_area, state, sizeof(state_t));
        LDST(areas->new_area);
    }
    else
    {
        //In caso di errore termina il processo
        terminateCurrentProcess();
        schedule();
    }
}

//Handler per system call e breakpoint
void handler_sysbk(void)
{
    pcb_t* process = getCurrentProcess();
    if(!process) 
    {
        kprintf("syscall with no current process!\n");
        PANIC();
    }
    
    //Aggiorna l'user_time al timestamp attuale - il quello all'inizio dell'ultima time slice
    process->user_time += getTime() - getTimeSliceBegin();
    unsigned int start_kernel_time = getTime();
     
    state_t* old_state = (state_t*)SYSBK_OLDAREA;
#ifdef TARGET_UMPS
    //Incrementa il pc di 4 per l'architettura umps
    PC(old_state) += 4;
#endif
    
    int exc_code = STATE_EXCCODE(old_state);
    
    //Gestisce il caso di una syscall
    if(exc_code == EXC_SYSCALL)
    {
        //Numero della syscall
        unsigned int syscall_number = STATE_SYSCALL_NUMBER(old_state);
        
        //Parametri della syscall
        unsigned int p1 = STATE_SYSCALL_P1(old_state);
        unsigned int p2 = STATE_SYSCALL_P2(old_state);
        unsigned int p3 = STATE_SYSCALL_P3(old_state);
        
        switch(syscall_number)
        {
            case GETCPUTIME:       syscallGetCPUTime((unsigned int*)p1, (unsigned int*)p2, (unsigned int*)p3);                 break;
            case CREATEPROCESS:    STATE_SYSCALL_RETURN(old_state) = syscallCreateProcess((state_t*)p1, (int)p2, (void **)p3); break;
            case TERMINATEPROCESS: STATE_SYSCALL_RETURN(old_state) = syscallTerminateProcess((void *)p1);                      break;
            case VERHOGEN:         syscallVerhogen((int*)p1);                                                                  break;
            case PASSEREN:         syscallPasseren((int*)p1);                                                                  break;
            case WAITIO:           syscallDo_IO(p1, (unsigned int*)p2, (int)p3);                                               break;
            case SPECPASSUP:       STATE_SYSCALL_RETURN(old_state) = syscallSpecPassup((int)p1, (state_t*)p2, (state_t*)p3);   break;
            case GETPID:           syscallGetPidPPid((void**)p1, (void**)p2);                                                  break;
            
            default:
            {
                //Chiama l'handler della specpassup se il codice non e' di una system call del kernel
                callSpecpassupHandler(old_state, SPECPASSUP_TYPE_SYSBK);
            } break;
        }
    }
    else
    {
        //In caso di breakpoint chiama gli handler registrati dalla specpassup
        callSpecpassupHandler(old_state, SPECPASSUP_TYPE_SYSBK);
    }
    
    //Aggiorna il tempo in kernel mode del processo che ha causato la system call
    process->kernel_time += getTime() - start_kernel_time;
    
    //Se il processo corrente non e' stato sospeso o terminato prosegue l'esecuzione
    //dopo aver aggiornato l'inizio dello user time
    if(getCurrentProcess())
    {
        updateTimeSliceBegin();
        LDST(old_state);
    }
    //Altrimenti aggiorniamo il suo stato e scheduliamo il prossimo processo
    else
    {
        copy_memory(&process->p_s, old_state, sizeof(state_t));
        schedule();
    }
}

//Handler per program traps
void handler_pgmtrap(void)
{
    state_t* old_state = (state_t*)PGMTRAP_OLDAREA;
    
    //Chiama gli handler registrati dalla specpassup o termina il processo in caso di errore
    callSpecpassupHandler(old_state, SPECPASSUP_TYPE_PGMTRAP);
}

//Handler per la gestione del TLB
void handler_tlb(void)
{
    state_t* old_state = (state_t*)TLB_OLDAREA;
    
    //Chiama gli handler registrati dalla specpassup o termina il processo in caso di errore
    callSpecpassupHandler(old_state, SPECPASSUP_TYPE_TLB);
}

//Handler per la gestione degli interrupt
void handler_int(void)
{
    state_t* old_state = (state_t*)INT_OLDAREA;
    
    unsigned int interrupt_begin_timestamp = getTime();
    
#ifdef TARGET_UARM
    //Come specificato al capitolo 2.5.3 del manuale di uArm il pc va decrementato di 4
    //in seguito ad un interrupt
    old_state->pc -= 4;
#endif
    
    unsigned int cause = STATE_CAUSE(old_state);
    
    int resumed_max_priority = checkDeviceInterrupts(cause);
    
    if(CAUSE_IP_GET(cause, IL_TIMER))
    {
        //Se il time slice e' terminato in seguito a un interrupt dell'interval timer
        //aggiorna lo stato del processo corrente e reinvoca lo scheduler
        pcb_t* current_proc = getCurrentProcess();
        updateCurrentProcess(old_state);
        
        //Il time slice in user mode termina definitivamente e l'user time viene aggiornato
        current_proc->user_time += interrupt_begin_timestamp - getTimeSliceBegin();
        
        //la chiamata schedule() non ritorna
        schedule();
    }
    else
    {
        pcb_t* current_proc = getCurrentProcess();
        
        //Se il processo di priorita' massima tra quelli svegliati da un interrupt
        //ha priorita' piu' alta del processo corrente passiamo il controllo ad esso
        if(resumed_max_priority > current_proc->priority)
        {
            //Aggiorniamo il suo pcb con lo stato attuale e il tempo user trascorso
            updateCurrentProcess(old_state);
            current_proc->user_time += interrupt_begin_timestamp - getTimeSliceBegin();
            
            schedule();
        }
        else
        {
            //Altrimenti riprendi l'esecuzione del processo precedente
            LDST(old_state);
        }
    }
}

