#include "pcb.h"
#include "asl.h"
#include "const.h"

static struct list_head semdFreeList;
static semd_t semd_table[MAXPROC];
static struct list_head ASL;

/* ASL handling functions */

/* Inizializzazione delle liste ASL e semdFreeList */
void initASL(void) 
{

	INIT_LIST_HEAD(&ASL);
    INIT_LIST_HEAD(&semdFreeList);
    for(int i = 0; i < MAXPROC; i++)
    {
        list_add(&semd_table[i].s_next, &semdFreeList);
    }
}

/* Restituisce il semd associato alla chiave data in input */
semd_t* getSemd(int *key) 
{
	/*Scorre la lista di semd utilizzati finchè non c'è una corrispondenza, 
      se non si trova e il ciclo for termina ritorna NULL*/
    semd_t* pos;
    list_for_each_entry(pos, &ASL, s_next)
    {
        if(pos->s_key==key) return pos;
    }
    
	return NULL;
}

/* Prende, se disponibile, un semd dalla lista di quelli liberi e lo restituisce 
   dopo aver inizializzato i campi*/
static semd_t *allocSemd(void) 
{
	struct list_head* tmp = list_next(&semdFreeList);
	
	if(tmp==NULL)
	{
		return NULL;
	}
	else
	{
		list_del(tmp);
		
		//Inizializzazione dei campi
		semd_t* result = container_of(tmp,semd_t,s_next);
		INIT_LIST_HEAD(&result->s_next);
		INIT_LIST_HEAD(&result->s_procQ);
		result->s_key=NULL;
		return(result);
	}
}


/*Restituisce un semd dalla ASL alla lista dei semd liberi*/
void freeSemd(semd_t *toDel) 
{
	list_del(&toDel->s_next);
	list_add(&toDel->s_next,&semdFreeList);
}

/*Inserisce un pcb p nella coda di processi associata al semaforo con chiave key*/
int insertBlocked(int *key, pcb_t* p) 
{
	semd_t* tmp=getSemd(key);
	/*Se esite aggiunge il pcb altrimenti crea un nuovo semd*/
	if(tmp!=NULL)
	{
		p->p_semkey=key;
		insertProcQ(&tmp->s_procQ, p);
		return 0;
	}
	else
	{
		semd_t* newSemd = allocSemd();
		/*Verifica l'esistenza di un semd disponibile nella freeList*/
		if(newSemd==NULL)
		{
			return 1;
		}
		else
		{
			p->p_semkey=key;
			newSemd->s_key=key;
			mkEmptyProcQ(&newSemd->s_procQ);
			insertProcQ(&newSemd->s_procQ,p);
			list_add_tail(&newSemd->s_next,&ASL);
			return 0;
		}
	}
}

/*Rimuove il primo pcb in coda al semd con chiave key*/
pcb_t* removeBlocked(int *key) 
{
	semd_t* tmp = getSemd(key);
	/*Nel caso non esista un semd con chiave key ritorna NULL*/
	if (tmp==NULL)
	{
		return NULL;
	}
	else
	{
		pcb_t* result = removeProcQ(&tmp->s_procQ);
		result->p_semkey=NULL;
        
		/*Se la lista è diventata vuota libero il semd */
		if (emptyProcQ(&tmp->s_procQ))
		{
			freeSemd(tmp);
		}
		return result;
	}
}

/*Restituisce e rimuove un pcb dalla coda dei processi del semaforo su cui e' bloccato */
pcb_t* outBlocked(pcb_t *p) 
{
	semd_t* tmp = getSemd(p->p_semkey);
	/*Nel caso non esista un semd con chiave key ritorna NULL*/
	if (tmp==NULL)
	{
		return NULL;
	}
	else
	{
		pcb_t* result = outProcQ(&tmp->s_procQ,p);
		result->p_semkey=NULL;
		
		/*Se la lista è diventata vuota libera il semd */
		if (emptyProcQ(&tmp->s_procQ))
		{
			freeSemd(tmp);
		}
		return result;
	}
}

/*Ritorna il primo pcb bloccato sul semaforo con chiave key nell'ASL, 
  se non esiste ritorna NULL*/
pcb_t* headBlocked(int *key) 
{
	semd_t* tmp = getSemd(key);
	if(tmp==NULL)
	{
		return NULL;
	}
	else
	{
		pcb_t* result = headProcQ(&tmp->s_procQ);
		return result;
	}
}

/*Rimuove il pcb p dal semaforo su cui e' bloccato e procede 
  ricorsivamente su tutti i figli*/
void outChildBlocked(pcb_t *p) 
{
	
	outBlocked(p);
	pcb_t* pos;
	list_for_each_entry(pos,&p->p_child,p_sib)
	{
		outChildBlocked(pos);
	}
}


