#include "scheduler.h"
#include "system.h"

static struct list_head ready_queue;
static pcb_t* current_process;

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

static void switchToProcess(pcb_t* p)
{
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
}
