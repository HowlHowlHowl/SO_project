#include "kprintf.h"
#include "system.h"
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
    
    //Se la causa dell'interrupt e' il IL_TIMER
    if(CAUSE_IP_GET(cause, IL_TIMER))
    {
        //Aggiorna lo stato del processo corrente e reinvoca lo scheduler,
        //la chiamata schedule() non ritorna
        updateCurrentProcess(old_state);
        schedule();
    }
    else
    {
        kprintf("Unexpected interrupt, cause register: 0x%x\n", cause);
    }
    
    LDST(old_state);
}

