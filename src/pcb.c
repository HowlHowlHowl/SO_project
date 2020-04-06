#include "const.h"
#include "pcb.h"
#include "utils.h"

/* PCB free list e table */
static struct list_head pcbFreeList;
static pcb_t pcbFreeTable[MAXPROC];

/* PCB free list handling functions */

/* Inizializza la lista libera in modo che contenga tutti i pcb disponibili */
void initPcbs(void)
{
    INIT_LIST_HEAD(&pcbFreeList);
    
    for(int i = 0; i < MAXPROC; i++)
    {
        list_add(&pcbFreeTable[i].p_next, &pcbFreeList);
    }
}

/* Libera il pcb puntato da p aggiungendolo alla lista libera */
void freePcb(pcb_t *p)
{
    list_add(&p->p_next, &pcbFreeList);
}

/* Alloca un pcb dalla lista libera se disponibile, altrimenti ritorna NULL*/
pcb_t *allocPcb(void)
{
    struct list_head* next = list_next(&pcbFreeList);

    if(!next)
        return NULL;

    list_del(next);
    
    pcb_t* result = container_of(next, pcb_t, p_next);
    
    /*Initializza tutti gli elementi del pcb */
    INIT_LIST_HEAD(&result->p_next);
    result->p_parent = NULL;
    INIT_LIST_HEAD(&result->p_child);
    INIT_LIST_HEAD(&result->p_sib);
    zero_memory(&result->p_s, sizeof(state_t));
    result->priority = 0;
    result->original_priority = 0;
    result->p_semkey = NULL;
    
    return result;
}

/* PCB queue handling functions */

/*Inizialliza head come coda vuota*/
void mkEmptyProcQ(struct list_head *head)
{
    INIT_LIST_HEAD(head);
}

/*Ritorna non-zero se la coda e' vuota, zero altrimenti*/
int emptyProcQ(struct list_head *head)
{
    return list_empty(head);
}

/*Inserisce p nella coda puntata da head*/
void insertProcQ(struct list_head *head, pcb_t *p)
{
    if(list_empty(head))
    {
        list_add(&p->p_next, head);
    }
    else
    {
        /*Se non troviamo un elemento prima del quale inserire p lo inseriamo alla fine
          della coda  */
        struct list_head* prev = head->prev;
        
        /*Scorre la coda alla ricerca di un elemento prima del quale inserire p*/
        pcb_t* it;
        list_for_each_entry(it, head, p_next)
        {
            if(p->priority >= it->priority)
            {
                prev = it->p_next.prev;
                break;
            }
        }
        
        /*Inserisce p subito dopo prev */
        list_add(&p->p_next, prev);
    }
}

/*Ritorna il primo elemento della coda puntata da head, se e' vuota ritorna NULL*/
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

/*Rimuove e ritorna il primo elemento della coda puntata da head, se e' vuota ritorna NULL*/
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

/*Rimuove e ritorna p dalla coda puntata da head, se non e' presente ritorna NULL*/
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

/*Ritorna non-zero se p ha figli, zero altrimenti*/
int emptyChild(pcb_t *p)
{
    return list_empty(&p->p_child);
}

/*Inserisce p come figlio di prnt*/
void insertChild(pcb_t *prnt, pcb_t *p)
{
    list_add_tail(&p->p_sib, &prnt->p_child);
    p->p_parent = prnt;
}

/*Rimuove e ritorna il primo figlio di p, se p non ha figli ritorna NULL*/
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

/*Rimuove p dai figli del padre, se p non ha padre ritorna NULL*/
pcb_t *outChild(pcb_t *p)
{
    if(p->p_parent)
    {
        list_del(&p->p_sib);
        p->p_parent = 0;
        return p;
    }
    else
    {
        return NULL;
    }
}


