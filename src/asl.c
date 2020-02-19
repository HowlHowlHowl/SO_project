#include "pcb.h"
#include "asl.h"
#include "const.h"



static struct list_head semdFreeList;
static semd_t semd_table[MAXPROC];
static struct list_head ASL;

/* ASL handling functions 

Inizializzazione delle liste ASL e semdFree */
void initASL() {

	INIT_LIST_HEAD(&ASL);
    INIT_LIST_HEAD(&semdFreeList);
    for(int i = 0; i < MAXPROC; i++)
    {
        list_add(&semd_table[i].s_next, &semdFreeList);
    }
}

/*Aggiunge un semd alla ASL rimuovendolo dalla semdFreeList */
void AslAdd(semd_t* newOne) {

	list_del(&newOne->s_next);
	list_add_tail(&newOne->s_next,&ASL);	
}



/* Restituisce il semd associato alla chiave data in input */
semd_t* getSemd(int *key) {

	/*Scorre la lista di semd utilizzati finchè non c'è una corrispondenza, se non si trova e il ciclo for termina oppure la key in input e nulla ritorna NULL*/
	if (key)
	{
		struct list_head* tmp = &ASL; 
		semd_t* pos;
		list_for_each_entry(pos,tmp,s_next) 
		{
			if(pos->s_key==key) return pos;	
		}
	}
	return NULL;
}
/* Prende, se disponibile, un semd dalla lista di quelli liberi e lo restituisce come output dopo aver inizializzato i campi*/
semd_t *allocSemd (void) {

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
void freeSemd(semd_t *toDel) {
	toDel->s_key=NULL;
	list_del(&toDel->s_next);
	list_add(&toDel->s_next,&semdFreeList);
}


int insertBlocked(int *key, pcb_t* p) {

	semd_t* tmp=getSemd(key);
	/*Esistenza di un semd con chiave == key all'interno della ASL, se esite aggiungo il pcb altrimenti creo un nuovo semd*/
	if(tmp!=NULL) 
	{	
		p->p_semkey=key;
		insertProcQ(&tmp->s_procQ, p);
		return 0;
	}
	else 
	{
		semd_t* newSemd = allocSemd();
		
		/*Verifica dell'esistenza di un semd disponibile nella freeList*/
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

/*Rimozione del primo pcb in coda s_ProcQ associato al semd con s_key == key*/
pcb_t* removeBlocked(int *key) {

	semd_t* tmp = getSemd(key);
	/*Esistenza di un semd con chiave == key, nel caso non esista (=> tmp==NULL) ritorna NULL*/
	if (tmp==NULL) 
	{
		return NULL;
	} 
	else 
	{
		//Assegno il pcb eliminato se la lista è diventata vuota libero il semd altrimenti mi limito a ritornare
		pcb_t* result = removeProcQ(&tmp->s_procQ);
		if (emptyProcQ(&tmp->s_procQ)) 
		{
			freeSemd(tmp);
		}
		result->p_semkey=NULL;
		return result;
	}
}

/*Restituire e Rimuovere un pcb specifico dalla lista.
	Ricerca di un semd con key_s==p->p_semkey, se non esiste ritorno NULL
	Estrazione del pcb in questine. Con outProcQ verifico se esiste un altro 
	elemento nella semd corrispondente se non esiste libero la semd
*/
pcb_t* outBlocked(pcb_t *p) {

	semd_t* tmp = getSemd(p->p_semkey);
	if (tmp==NULL) 
	{
		return NULL;
	} 
	else  
	{
		pcb_t* result = outProcQ(&tmp->s_procQ,p);
		
		if (headProcQ(&tmp->s_procQ))
		{
			freeSemd(tmp);	
		}
		result->p_semkey=NULL;
		return result;
	}
}

/*Esistenza di un semd con chiave == key all'interno dell'ASL, se non esiste ritorna NULL, per ritornare l'elemento di testa della lista dei pbc chiama la funzione headProcQ (file 'pbc.c') */
pcb_t* headBlocked(int *key) {

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

/*Rimuove il pcb p iniziale e procede iterando su tutti i figli di p il procedimento ricorsivo*/
void outChildBlocked(pcb_t *p) {
	
	outBlocked(p);
	pcb_t* pos;
	list_for_each_entry(pos,&p->p_child,p_sib)
	{
		outChildBlocked(pos);
	}
}


