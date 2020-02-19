#include "const.h"
#include "pcb.h"

/* PCB List and table data */
static struct list_head pcbFreeList;
static pcb_t pcbFreeTable[MAXPROC];

/* Sets "size" bytes starting from "ptr" to 0 */
static void zero_memory(void* ptr, unsigned int size)
{
    char* p = (char*)ptr;
    for(unsigned int i = 0; i < size; i++)
    {
        p[i] = 0;
    }
}

/* PCB free list handling functions */
void initPcbs(void)
{
    INIT_LIST_HEAD(&pcbFreeList);
    
    for(int i = 0; i < MAXPROC; i++)
    {
        list_add(&pcbFreeTable[i].p_next, &pcbFreeList);
    }
}

void freePcb(pcb_t *p)
{
    list_add(&p->p_next, &pcbFreeList);
}

pcb_t *allocPcb(void)
{
    struct list_head* next = list_next(&pcbFreeList);

    if(!next)
        return NULL;

    list_del(next);
    
    pcb_t* result = container_of(next, pcb_t, p_next);
    
    //Initialize all fields of result
    INIT_LIST_HEAD(&result->p_next);
    result->p_parent = NULL;
    INIT_LIST_HEAD(&result->p_child);
    INIT_LIST_HEAD(&result->p_sib);
    zero_memory(&result->p_s, sizeof(state_t));
    result->priority = 0;
    result->p_semkey = NULL;
    
    return result;
}

/* PCB queue handling functions */
void mkEmptyProcQ(struct list_head *head)
{
    INIT_LIST_HEAD(head);
}

int emptyProcQ(struct list_head *head)
{
    return list_empty(head);
}

void insertProcQ(struct list_head *head, pcb_t *p)
{
    if(list_empty(head))
    {
        list_add(&p->p_next, head);
    }
    else
    {
        /* 
           Se non troviamo un elemento prima del quale inserire p lo inseriamo alla fine
           della lista 
        */
        struct list_head* prev = head->prev;
        pcb_t* it;
        list_for_each_entry(it, head, p_next)
        {
            if(it->priority <= p->priority)
            {
                prev = it->p_next.prev;
                break;
            }
        }
        
        list_add(&p->p_next, prev);
    }
}

pcb_t *headProcQ(struct list_head *head)
{
    struct list_head* first = list_next(head);
    if(first)
    {
        return container_of(first, pcb_t, p_next);
    }
    else
    {
        return NULL;
    }
}

pcb_t *removeProcQ(struct list_head *head)
{
    struct list_head* first = list_next(head);
    if(first)
    {
        list_del(first);
        return container_of(first, pcb_t, p_next);
    }
    else
    {
        return NULL;
    }
}

pcb_t *outProcQ(struct list_head *head, pcb_t *p)
{
    pcb_t* result = NULL;
    pcb_t* it;
    list_for_each_entry(it, head, p_next)
    {
        if(p == it)
        {
            list_del(&it->p_next);
            result = it;
            break;
        }
    }
    
    return result;
}


/* Tree view functions */
int emptyChild(pcb_t *this)
{
    return list_empty(&this->p_child);
}

void insertChild(pcb_t *prnt, pcb_t *p)
{
    list_add_tail(&p->p_sib, &prnt->p_child);
    p->p_parent = prnt;
}

pcb_t *removeChild(pcb_t *p)
{
    struct list_head* first_child = list_next(&p->p_child);
    if(first_child)
    {
        list_del(first_child);
        return container_of(first_child, pcb_t, p_sib);
    }
    else
    {
        return NULL;
    }
}

pcb_t *outChild(pcb_t *p)
{
    if(p->p_parent)
    {
        list_del(&p->p_sib);
        return p;
    }
    else
    {
        return NULL;
    }
}


