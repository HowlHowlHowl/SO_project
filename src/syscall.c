#include "system.h"
#include "scheduler.h"
#include "asl.h"
#include "syscall.h"

//Vengono definiti i registri da modificare se l'operazione 
//SYSCALL_CREATE_PROCESS (sysCallCreateProcess();) va a buon  fine
#ifdef TARGET_UMPS
	#define STATUS(s) (s)->status
	#define PC(s) (s)->pc_epc
	#define SP(s) (s)->reg_sp
#endif
//Analogamente per UARM
#ifdef TARGET_UARM
	#define STATUS(s) (s)->cpsr
	#define PC(s) (s)->pc
	#define SP(s) (s)->sp
#endif
	
	
//SysCall 1. 
//Quando invocata restituisce il tempo di esecuzione del processo chiamante
// suddiviso in user_time kernel_time e global_time, 
//il tempo totale trasccorso dalla prima attivazione del processo.
void sysCallGetCPUTime(unsigned int* user, unsigned int* kernel, unsigned int *wallclock)
{
	pcb_t* currentProc = getCurrentProcess();
	*user = currentProc->user_time;
	*kernel = currentProc->kernel_time;
	*wallclock = (getTime() - currentProc->begin_timestamp);
}

//SysCall 2. Crea un nuovo processo come figlio del chiamante, 
//se la sysCall ha successo cpid continene l'indirizzo del pcb_t del processo figlio creato
int sysCallCreateProcess(state_t* statep, int priority, void **cpid)
{
	pcb_t* childProc = allocPcb();
	//Controllare priority != PRIO_IDLE?
	if(childProc)
	{
		pcb_t* currentProc = getCurrentProcess();
		insertChild(currentProc, childProc);
		if(cpid)
		{
			*cpid = childProc;
		}
		childProc->priority = priority;
		
		state_t* status = &childProc->p_s;
		STATUS(status) = STATUS(statep);
		PC(status) = PC(statep);
		SP(status) = SP(statep);
		addProcess(childProc,priority);
		return 0;
	}
	//Non c'è il ramo else così il compilatore è contento
	return -1;
}

//SysCall 3. Termina il processo identificato dal parametro pid 
//e l'albero di processi radicato in esso, se non esiste ritorrna errore. 
//Se pid è NULL termina il processo corrente
int sysCallTerminateProcess(void* pid,int a, int b){
	if(!pid)
	{
		terminateCurrentProcess();
		schedule();
		return 0;
	}
	else 
	{	pcb_t* PCB = (pcb_t*) pid; 
		if(PCB)
		{
			terminateProcess(PCB);
			return 0;
		}
		else
		{
			return -1;
		}
	}
}

//SysCall 4. Operazione di rilascio sul semaforo identificato dal valore di semaddr ::= V() sui semafori
void sysCallVerhogen(int* semaddr,int a, int b)
{
	//disabilitare gli interrupt?
	semd_t* sem = getSemd(semaddr);
	if(emptyProcQ(&sem->s_procQ))
	{
		*semaddr+=1;
	}
	else
	{
	resumeProcess(removeBlocked((semaddr)));
	}
}

//SysCall 5. Operazione di richiesta di un semaforo ::= P() sui semafori
void sysCallPasseren(int* semaddr, int a, int b)
{
	//val mem in *semaddr --- semaddr identifica il sem
	if(*semaddr==0)
	{
		pcb_t* process = getCurrentProcess();
		insertBlocked(semaddr, &process);
		removeCurrentProcess();
	}
	else
	{
		*semaddr-=1;
	}
	
}
//SysCall 6.
void sysCallIO(unsigned int command, unsigned int *reg, int subdevice)
{
	//Viene definito un device generico
	devreg_t* dev = (devreg_t*)reg; 
	//Il processo corrente viene rimosso dalla ready_queue 
	pcb_t* currentProcess = removeCurrentProcess();
	int *key = (int*)reg;
	//Identificazione device
	//TODO: gestione errori di identificazione device
	if((unsigned int)reg < DEV_REG_ADDR(IL_TERMINAL,0))
	{
		dev->dtp.command = command;
	}
	else 
	{
	//Identificazione del subdevice nel caso la SYSCALL sia stata effettuata da un terminale
		if(subdevice)
		{
			dev->term.recv_command = command;
			key =(int*) &dev->term.recv_status;
		}
		else
		{
			dev->term.transm_command = command;
			key = (int*) &dev->term.transm_status;
		}
	}
	//Inserimento del processo corrente in coda su di un semaforo 
	insertBlocked(key, currentProcess);
	schedule();
}