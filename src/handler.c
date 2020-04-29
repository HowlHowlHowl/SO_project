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
    if(!p)
    {
        kprintf("Current process is NULL!\n");
        PANIC();
    }
    
    handler_areas* areas = &p->specpassup_areas[type];
    if(areas->old_area && areas->new_area)
    {
        kprintf("Process %x calling registered specpassuap area of type %d: new = %x old = %x\n", p, type, areas->old_area, areas->new_area);
        //Salva lo stato corrente nella old area e passa il controlla alla new area
        copy_memory(areas->old_area, state, sizeof(state_t));
        LDST(areas->new_area);
    }
    else
    {
        kprintf("Process %x terminated due to call to non-registered specpassup areas, type: %d\n", p, type);
        
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
    
    //Aggiorna l'user_time al tempo attuale - il tempo di inizio dell'ultima time slice
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
        
//        kprintf("Syscall from process %x with number: %u\n", process, syscall_number);
        
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
                callSpecpassupHandler(old_state, SPECPASSUP_TYPE_SYSBK);
            } break;
        }
    }
    else
    {
        kprintf("Breakpoint\n");
        
        //In caso di breakpoint chiama gli handler registrati dalla specpassup
        callSpecpassupHandler(old_state, SPECPASSUP_TYPE_SYSBK);
    }
    
    process->kernel_time += getTime() - start_kernel_time;
    
    if(getCurrentProcess())
    {
        //Se il processo corrente non e' stato sospeso o terminato
        //prosegue l'esecuzione
        updateTimeSliceBegin();
        LDST(old_state);
    }
    else
    {
        //Altrimenti aggiorniamo il suo stato e scheduliamo il prossimo processo
        copy_memory(&process->p_s, old_state, sizeof(state_t));
        schedule();
    }
}

//Handler stub per program traps
void handler_pgmtrap(void)
{
    state_t* old_state = (state_t*)PGMTRAP_OLDAREA;
    
    //Chiama gli handler registrati dalla specpassup
    callSpecpassupHandler(old_state, SPECPASSUP_TYPE_PGMTRAP);
}

//Handler stub per la gestione del TLB
void handler_tlb(void)
{
    state_t* old_state = (state_t*)TLB_OLDAREA;
    
    //Chiama gli handler registrati dalla specpassup
    callSpecpassupHandler(old_state, SPECPASSUP_TYPE_TLB);
}

//Handler per la gestione degli interrupt
void handler_int(void)
{
    state_t* old_state = (state_t*)INT_OLDAREA;
    if(getCurrentProcess() == NULL)
    {
        kprintf("Interrupt with no current process!\n");
        PANIC();
    }
#ifdef TARGET_UARM
    //Come specificato al capitolo 2.5.3 del manuale di uArm il pc va decrementato di 4
    //in seguito ad un interrupt
    old_state->pc -= 4;
#endif
    
    unsigned int cause = STATE_CAUSE(old_state);
    
    int any_resumed = checkDeviceInterrupts(cause);
    
    if(CAUSE_IP_GET(cause, IL_TIMER))
    {
        //Se il time slice e' terminato in seguito a un interrupt dell'interval timer
        //aggiorna lo stato del processo corrente e reinvoca lo scheduler,
        pcb_t* current_proc = getCurrentProcess();
        updateCurrentProcess(old_state);
        
        //Il time slice in user mode termina definitivamente e l'user time viene aggiornato
        current_proc->user_time += getTimeSliceBegin() - getTime();
        
        //la chiamata schedule() non ritorna
        schedule();
    }
    else
    {
        if(any_resumed && isIdleProcessCurrent())
        {
            //Se il processo corrente e' l'idle e ci sono processi svegliati in seguito
            //a un interrupt interrompiamo il processo corrente e passiamo il controllo
            //a quello di priorita' piu' alta.
            updateCurrentProcess(old_state);
            schedule();
        }
        else
        {
            //Altrimenti riprendi l'esecuzione del processo precedente
            LDST(old_state);
        }
    }
}

