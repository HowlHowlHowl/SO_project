#include "system.h"
#include "pcb.h"
#include "kprintf.h"
#include "scheduler.h"
#include "utils.h"
#include "handler.h"

void test1();
void test2();
void test3();

void initNewArea(unsigned int area_addr, void (*handler)(void))
{	//Allocate the new area
	state_t* state = (state_t*)area_addr;
    zero_memory(state, sizeof(state_t));
    
    /*Set the status register to state as below: 
      interrupts masked - virtual_memory OFF - kernel_mode ON.
      Also:
      Set the PC to the dedicated handler for each new area
      Set the SP to RamTop */
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

pcb_t* initPcbState(unsigned int stack_pointer, void (*func)(void)){
	pcb_t* p = allocPcb();
    /*Enable kernel mode and interval timer interrupt line 
      Set the PC to point the address of the relative test function*/
#ifdef TARGET_UARM
    p->p_s.sp = stack_pointer;
    p->p_s.pc = (unsigned int)func;
    p->p_s.cpsr = STATUS_SYS_MODE;
    p->p_s.cpsr = STATUS_ALL_INT_DISABLE(p->p_s.cpsr);
    p->p_s.cpsr = STATUS_ENABLE_TIMER(p->p_s.cpsr);
#endif
    
#ifdef TARGET_UMPS
    p->p_s.reg_sp= stack_pointer;
    p->p_s.pc_epc = (unsigned int)func;
    p->p_s.status = STATUS_CU0 | STATUS_IM(IL_TIMER) | STATUS_IEp;
#endif
    
    return p;
}

void idle_func(void)
{
    kprintf("Begin idle process\n");
    while(1)
    {
        WAIT();
    }
}

int main()
{
    initPcbs();
    initScheduler();
    //Init of all the four new areas
    initNewArea(SYSBK_NEWAREA,   handler_sysbk);
    initNewArea(PGMTRAP_NEWAREA, handler_pgmtrap);
    initNewArea(TLB_NEWAREA,     handler_tlb);
    initNewArea(INT_NEWAREA,     handler_int);
    //Init of the three test processes
    pcb_t* p1 = initPcbState(RAMTOP - FRAME_SIZE * 1, test1);
    pcb_t* p2 = initPcbState(RAMTOP - FRAME_SIZE * 2, test2);
    pcb_t* p3 = initPcbState(RAMTOP - FRAME_SIZE * 3, test3);
    //Adding the processes into the scheduler
    addProcess(p1, 1);
    addProcess(p2, 2);
    addProcess(p3, 3);
    //Adding the idle process with the lowest priority
    pcb_t* idle = initPcbState(RAMTOP - FRAME_SIZE * 4, idle_func);
    setIdleProcess(idle);
    /*Find the next process to execute and giving it the control and
     refresh the priority of each remaining process*/
    schedule();
    
    return 1;
}
