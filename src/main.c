#include "system.h"
#include "pcb.h"
#include "kprintf.h"
#include "scheduler.h"
#include "utils.h"

void test1();
void test2();
void test3();

void initNewArea(unsigned int area_addr, unsigned int handler_addr)
{
	state_t* state = (state_t*)area_addr;
    zero_memory(state, sizeof(state_t));
    
    /*set the status register to t state: 
      interrupts masked - virtual_memory OFF - kernel_mode ON*/
#ifdef TARGET_UARM
    state->sp = RAMTOP;
    state->pc = handler_addr;
    state->CP15_Control = 0;
    state->cpsr = STATUS_SYS_MODE;
    state->cpsr = STATUS_ALL_INT_DISABLE(state->cpsr);
#endif
    
#ifdef TARGET_UMPS
    state->reg_sp = RAMTOP;
    state->pc_epc = handler_addr;
    state->status = STATUS_KUc;
#endif
    
}

void initPcbState(int priority, unsigned int stack_pointer, void (*test_fun)()){
	pcb_t* test = allocPcb();
	
    //Enable kernel mode and interval timer interrupt line
#ifdef TARGET_UARM
    test->p_s.sp = stack_pointer;
    test->p_s.pc = (unsigned int)test_fun;
    test->p_s.cpsr= STATUS_SYS_MODE;
    test->p_s.cpsr= STATUS_ENABLE_TIMER(test->p_s.cpsr);
#endif
    
#ifdef TARGET_UMPS
    test->p_s.reg_sp= stack_pointer;
    test->p_s.pc_epc = (unsigned int)test_fun;
    test->p_s.status = STATUS_KUc | STATUS_IM(IL_TIMER); //STATUS_IM_MASK | STATUS_TE | STATUS_IEp ;
#endif
    
	addProcess(test, priority);
}


int main()
{
    initPcbs();
    initScheduler();
    
    initNewArea(SYSBK_NEWAREA,   0);
    initNewArea(PGMTRAP_NEWAREA, 0);
    initNewArea(TLB_NEWAREA,     0);
    initNewArea(INT_NEWAREA,     0);
    
    initPcbState(1, RAMTOP - FRAMESIZE * 1, test1);
    initPcbState(2, RAMTOP - FRAMESIZE * 2, test2);
    initPcbState(3, RAMTOP - FRAMESIZE * 3, test3);
    
    while(1)
    {
        WAIT();
    }
    
    return 1;
}
