#include "scheduler.h"
#include "system.h"
#include "kprintf.h"
#include "utils.h"

static struct list_head ready_queue;
static pcb_t* current_process;
static pcb_t* idle_process;

void initScheduler(void)
{
    mkEmptyProcQ(&ready_queue);
}

void addProcess(pcb_t* p, int priority)
{
    p->priority = priority;
    p->original_priority = priority;
    insertProcQ(&ready_queue, p);
}

static void removePcbRecursively(pcb_t* p)
{
    pcb_t* removed = outProcQ(&ready_queue, p);
    if(removed)
    {
        freePcb(removed);
    }
    
    pcb_t* pos;
	list_for_each_entry(pos, &p->p_child, p_sib)
	{
		removePcbRecursively(pos);
	}
}

void setIdleProcess(pcb_t* p)
{
    idle_process = p;
}

void updateCurrentProcess(state_t* state)
{
    copy_memory(&current_process->p_s, state, sizeof(state_t));
}

void terminateCurrentProcess(void)
{
    removePcbRecursively(current_process);
}

static void resetIntervalTimer(void)
{
    unsigned int time_scale = *(unsigned int*)BUS_REG_TIME_SCALE;
    //Imposta il timer a 3ms
    unsigned int timer_value = 3000 * time_scale;
    
    *(unsigned int*)BUS_REG_TIMER = timer_value;
}

static void switchToProcess(pcb_t* p)
{
    resetIntervalTimer();
    current_process = p;
    LDST(&p->p_s);
}

void schedule(void)
{
    
    pcb_t* first = removeProcQ(&ready_queue);
    if(first)
    {
        //Set the priority back to the original value
        first->priority = first->original_priority;

        //Update the priority of all other processes
        pcb_t* it;
        list_for_each_entry(it, &ready_queue, p_next)
        {
            it->priority++;
        }

        //Insert the process back into the queue
        insertProcQ(&ready_queue, first);
        
        switchToProcess(first);
    }
    
    switchToProcess(idle_process);
}
