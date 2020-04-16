#include "system.h"
#include "kprintf.h"
#include "scheduler.h"

//Macro per la gestione dei registri dello stato in modo analogo per entrambe le architetture

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


//Handler per system call e breakpoint
void handler_sysbk(void)
{
    state_t* old_state = (state_t*)SYSBK_OLDAREA;

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
