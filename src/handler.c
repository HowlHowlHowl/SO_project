#include "system.h"
#include "kprintf.h"
#include "const_bikaya.h"
#include "scheduler.h"
#include "systemcall.h"
//Macro per la gestione dei registri dello stato in modo analogo per entrambe le architetture

#ifdef TARGET_UMPS
#define STATE_EXCCODE(s) CAUSE_GET_EXCCODE((s)->cause)
#define STATE_SYSCALL_NUMBER(s) (s)->reg_a0
#define STATE_SYSCALL_P1(s) (s)->reg_a1
#define STATE_SYSCALL_P2(s) (s)->reg_a2
#define STATE_SYSCALL_P3(s) (s)->reg_a3
#define STATE_CAUSE(s) (s)->cause
#define STATE_SYSCALL_RETURN(s) (s)->reg_v0
//Definita in modo analogo a uarm
#define CAUSE_IP_GET(cause, n) ((cause) & (CAUSE_IP(n)))
#endif

#ifdef TARGET_UARM
#define STATE_EXCCODE(s) CAUSE_EXCCODE_GET((s)->CP15_Cause)
#define STATE_SYSCALL_NUMBER(s) (s)->a1
#define STATE_SYSCALL_P1(s) (s)->a2
#define STATE_SYSCALL_P2(s) (s)->a3
#define STATE_SYSCALL_P3(s) (s)->a4
#define STATE_SYSCALL_RETURN(s) (s)->a1
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
            case TERMINATEPROCESS:{
                // Terminiamo il processo  corrente e passiamo il controllo al prossimo
                // processo in coda, la chiamata schedule() non ritorna.
                terminateCurrentProcess();
                schedule();
                break;
            }
            case WAITIO:{
            	
				sysCallIO((unsigned int)STATE_SYSCALL_P1(old_state),(unsigned int*)STATE_SYSCALL_P2(old_state),(int)STATE_SYSCALL_P3(old_state));
				break;
            }
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
