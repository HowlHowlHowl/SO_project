#include "system.h"
#include "pcb.h"
#include "asl.h"
#include "scheduler.h"
#include "utils.h"
#include "handler.h"

void test();

//Popola la new area in area_addr
void initNewArea(unsigned int area_addr, void (*handler)(void))
{    
    state_t* state = (state_t*)area_addr;
    zero_memory(state, sizeof(state_t));
    
    /*Imposta lo status register allo stato seguente:
      interrupts mascherati - Virtual Memory OFF- Kernel Mode ON.
      Inoltre imposta:
      Il PC all handler dedicato alla new area corrispondente
      Lo SP a RAMTOP*/
#ifdef TARGET_UARM
    state->sp = RAMTOP;
    state->pc = (unsigned int)handler;
    state->CP15_Control = 0;
    state->cpsr = STATUS_SYS_MODE;
    state->cpsr = STATUS_ALL_INT_DISABLE(state->cpsr);
#endif
    
#ifdef TARGET_UMPS
    state->reg_sp = RAMTOP;
    state->pc_epc = (unsigned int)handler;
    state->status = STATUS_CU0;
#endif
    
}

//Inizializza un pcb e lo ritorna
pcb_t* initPcbState(unsigned int stack_pointer, void (*func)(void)){
    pcb_t* p = allocPcb();

    /*Abilita la kernel mode e tutte le interrupt line
      Imposta il PC a puntare l'address della relativa funzione di test*/
#ifdef TARGET_UARM
    p->p_s.sp = stack_pointer;
    p->p_s.pc = (unsigned int)func;
    p->p_s.cpsr = STATUS_SYS_MODE;
    p->p_s.cpsr = STATUS_ALL_INT_ENABLE(p->p_s.cpsr);
#endif
    
#ifdef TARGET_UMPS
    p->p_s.reg_sp = stack_pointer;
    p->p_s.pc_epc = (unsigned int)func;
    p->p_s.status = STATUS_CU0 | STATUS_IM_MASK | STATUS_IEp;
#endif
    
    return p;
}

void idle_func(void)
{
    while(1)
    {
        WAIT();
    }
}

int main()
{
    initPcbs();
    initASL();
    
    initScheduler();
    
    //Inizializzazione delle quattro new area
    initNewArea(SYSBK_NEWAREA,   handler_sysbk);
    initNewArea(PGMTRAP_NEWAREA, handler_pgmtrap);
    initNewArea(TLB_NEWAREA,     handler_tlb);
    initNewArea(INT_NEWAREA,     handler_int);
    
    pcb_t* p = initPcbState(RAMTOP - FRAME_SIZE, test);
    addProcess(p, 1);
    
    //Aggiunta del processo idle 
    pcb_t* idle = initPcbState(RAMTOP, idle_func);
    setIdleProcess(idle);
    
    //Avvio scheduler
    schedule();
    
    return 1;
}
