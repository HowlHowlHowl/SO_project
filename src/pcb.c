#include "pcb.h"

/* PCB free list handling functions */
void initPcbs(void) 
{
}

void freePcb(pcb_t *p) 
{
}

pcb_t *allocPcb(void) 
{ 
    return NULL;
}

/* PCB queue handling functions */
void mkEmptyProcQ(struct list_head *head) 
{
}

int emptyProcQ(struct list_head *head) 
{
    return 0;
}

void insertProcQ(struct list_head *head, pcb_t *p) 
{
}

pcb_t *headProcQ(struct list_head *head) 
{
    return NULL;
}

pcb_t *removeProcQ(struct list_head *head) 
{
    return NULL;
}

pcb_t *outProcQ(struct list_head *head, pcb_t *p) 
{
    return NULL;
}


/* Tree view functions */
int emptyChild(pcb_t *this) 
{
    return 0;
}

void insertChild(pcb_t *prnt, pcb_t *p) 
{
}

pcb_t *removeChild(pcb_t *p) 
{
    return NULL;
}

pcb_t *outChild(pcb_t *p) 
{
    return 0;
}


