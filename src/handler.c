#include "system.h"
#include "kprintf.h"
#include "const_bikaya.h"
#include "scheduler.h"
#include "syscall.h"
#include "interrupt.h"

//Handler per system call e breakpoint
void handler_sysbk(void)
{
    pcb_t* currentProc = getCurrentProcess();
    
    //Aggiorna l'user_time al tempo attuale - il tempo di inizio dell'ultima time slice
    currentProc->user_time += getTime() - getTimeSliceBegin();
    unsigned int startKernelTime = getTime();
     
    state_t* old_state = (state_t*)SYSBK_OLDAREA;
    updateCurrentProcess(old_state);
    
    //Vengono definiti i parametri per le SysCall
    unsigned int p1 = STATE_SYSCALL_P1(old_state);
    unsigned int p2 = STATE_SYSCALL_P2(old_state);
    unsigned int p3 = STATE_SYSCALL_P3(old_state);
    
#ifdef TARGET_UMPS
    //Incrementa il pc di 4 per l'architettura umps
    old_state->pc_epc += 4;
#endif
    
    int exc_code = STATE_EXCCODE(old_state);
    
    //Gestisce il caso di una syscall
    if(exc_code == EXC_SYSCALL)
    {
        unsigned int syscall_number = STATE_SYSCALL_NUMBER(old_state);
        
        switch(syscall_number)
        {
            case GETCPUTIME:       syscallGetCPUTime((unsigned int*)p1, (unsigned int*)p2, (unsigned int*)p3);                 break;
            case CREATEPROCESS:    STATE_SYSCALL_RETURN(old_state) = syscallCreateProcess((state_t*)p1, (int)p2, (void **)p3); break;
            case TERMINATEPROCESS: STATE_SYSCALL_RETURN(old_state) = syscallTerminateProcess((void *)p1);                      break;
            case VERHOGEN:         syscallVerhogen((int*)p1);                                                                  break;
            case PASSEREN:         syscallPasseren((int*)p1);                                                                  break;
            case WAITIO:           syscallDo_IO(p1, (unsigned int*)p2, (int)p3);                                               break;
            case GETPID:           syscallGetPidPPid((void**)p1, (void**)p2);                                                  break;
            
            default:
            {
                //TODO: Chiama lo specupassup handler invece di sta robba
                kprintf("Unexcpeted syscall identifier %u\n", syscall_number);
            } break;
        }
    }
    
    //TODO: pgmtrap if exc_code == user_mode for priviliged request
    currentProc->kernel_time += startKernelTime-getTime();
    updateTimeSliceBegin();
    
    LDST(old_state);
}

//Handler stub per program traps
void handler_pgmtrap(void)
{
    state_t* old_state = (state_t*)PGMTRAP_OLDAREA;
    
    kprintf("Unexpected pgmtrap exception\n");
    
    LDST(old_state);
}

//Handler stub per la gestione del TLB
void handler_tlb(void)
{
    state_t* old_state = (state_t*)TLB_OLDAREA;
    
    kprintf("Unexpected tlb exception\n");
    
    LDST(old_state);
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

