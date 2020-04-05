#include "main.h"
#include "pcb.h"
#include "kprintf.h"
#include "scheduler.h"
#define SYS_NEWAREA_UMPS		0x200003D4
#define PGMT_NEWAREA_UMPS		0x200002BC
#define TLB_NEWAREA_UMPS		0x200001A4
#define INT_NEWAREA_UMPS		0x2000008C
#define FRAMESIZE_UMPS 		 	1024
extern void test1();
extern void test2();
extern void test3();
static struct list_head* ready_queue;
int main()
{
   	initPcbs();
   	init_scheduler();
   	LIST_HEAD(ready_queue);
   	//initASL(); non è da fare in questa fase del progetto
   	#ifdef TARGET_UARM
   		init_newArea(SYSBK_NEWAREA,0); 
   		init_newArea(PGMTRAP_NEWAREA,0); 
   		init_newArea(TLB_NEWAREA,0); 
   		init_newArea(INT_NEWAREA,0); 
   	#endif
   	#ifdef TARGET_UMPS
   		init_newArea(SYS_NEWAREA_UMPS,0); 
   		init_newArea(PGMT_NEWAREA_UMPS,0); 
   		init_newArea(TLB_NEWAREA_UMPS,0); 
   		init_newArea(INT_NEWAREA_UMPS,0); 
   	#endif
   	init_Pcb_State(1, test1);
   	init_Pcb_State(2, test2);
   	init_Pcb_State(3, test3);
    while(1)
    {
        WAIT();
    }
    return 1;
}
void init_newArea(unsigned int areaAddr, unsigned int handlerAddr)
{
	state_t* state = (state_t*)areaAddr;
	#ifdef TARGET_UARM
		state->sp = ROMF_STACKTOP;
		state->pc = handlerAddr;
		state->CP15_Control = 0;
		state->cpsr=STATUS_SYS_MODE;
		state->cpsr=STATUS_ALL_INT_DISABLE(state->cpsr);
		/*this should set the current program status register to the boot state: interrupt_mask ON - virtual_memory OFF - kernel_mode ON*/
	#endif
	#ifdef TARGET_UMPS
		state->reg_sp=BUS_REG_RAM_BASE+BUS_REG_RAM_SIZE;
		state->pc_epc = handlerAddr;
		state->status = STATUS_KUc;
		/*as for uARM*/
	#endif
}
void init_Pcb_State(int priority, void test_fun()){
	pcb_t* test = allocPcb();
	
	#ifdef TARGET_UARM
		test->p_s.sp = ROMF_STACKTOP-(FRAMESIZE_UMPS*priority);
		test->p_s.pc = (unsigned int)test_fun;
		test->p_s.cpsr=STATUS_SYS_MODE;
		test->p_s.cpsr=STATUS_ENABLE_TIMER(test->p_s.cpsr);
	#endif
	#ifdef TARGET_UMPS
		test->p_s.reg_sp=(BUS_REG_RAM_BASE+BUS_REG_RAM_SIZE)-(FRAMESIZE_UMPS*priority);
		test->p_s.pc_epc = (unsigned int)test_fun;
		test->p_s.status = STATUS_KUc|STATUS_IEp|(STATUS_IM_MASK)|STATUS_TE;
	#endif
	add_process(test, priority);
	insertProcQ(ready_queue, test);
}
