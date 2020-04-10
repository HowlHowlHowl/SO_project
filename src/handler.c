#include "system.h"
#include "kprintf.h"
#include "scheduler.h"

#ifdef TARGET_UMPS
#define STATE_EXCCODE(s) CAUSE_GET_EXCCODE((s)->cause)
#define STATE_SYSCALL_NUMBER(s) (s)->reg_a0
#define STATE_CAUSE(s) (s)->cause
//Definita in modo analogo a uarm
#define CAUSE_IP_GET(cause, n) ((cause) & (CAUSE_IP(n)))
#endif

#ifdef TARGET_UARM
#define STATE_EXCCODE(s) CAUSE_EXCCODE_GET((s)->CP15_Cause)
#define STATE_SYSCALL_NUMBER(s) (s)->a1
#define STATE_CAUSE(s) (s)->CP15_Cause
#endif

void handler_sysbk(void)
{
    state_t* old_state = (state_t*)SYSBK_OLDAREA;

    int exc_code = STATE_EXCCODE(old_state);
    
    if(exc_code == EXC_SYSCALL)
    {
        unsigned int syscall_number = STATE_SYSCALL_NUMBER(old_state);
        
        switch(syscall_number)
        {
            case SYS_TERMINATEPROCESS:{
                // Terminiamo il processo  corrente e passiamo il controllo al prossimo
                // processo in coda, la chiamata schedule() non ritorna.
                terminateCurrentProcess();
                schedule();
            } break;
            
            default: kprintf("Unexcpeted syscall %u\n", syscall_number);
        }
    }
    
    LDST(old_state);
}

void handler_pgmtrap(void)
{
    state_t* old_state = (state_t*)PGMTRAP_OLDAREA;
    
    kprintf("Unexpected pgmtrap exception\n");
    
    LDST(old_state);
}

void handler_tlb(void)
{
    state_t* old_state = (state_t*)TLB_OLDAREA;
    
    kprintf("Unexpected tlb exception\n");
    
    LDST(old_state);
}

void handler_int(void)
{
    state_t* old_state = (state_t*)INT_OLDAREA;
    
#ifdef TARGET_UARM
    //Come specificato al capitolo 2.5.3 del manuale di uArm il pc va decrementato di 4
    //in seguito ad un interrupt
    old_state->pc -= 4;
#endif
    
    unsigned int cause = STATE_CAUSE(old_state);
    
    //Se la causa dell'interrupt e' il IL_TIMER
    if(CAUSE_IP_GET(cause, IL_TIMER))
    {
        updateCurrentProcess(old_state);
        schedule();
    }
    else
    {
        kprintf("Unexpected interrupt, cause register: 0x%x\n", cause);
    }
    
    LDST(old_state);
}