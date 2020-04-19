#include "system.h"
#include "scheduler.h"
#include "asl.h"
void sysCallTerminate(){
	terminateCurrentProcess();
	schedule();
}
void sysCallIO(unsigned int command, unsigned int *reg, int subdevice){
	//Viene definito un device generico
	devreg_t* dev = (devreg_t*)reg; 
	//Il processo corrente viene rimosso dalla ready_queue 
	pcb_t* currentProcess = removeCurrentProcess();
	int *key = (int*)reg;
	//Identificazione device
	if((unsigned int)reg<DEV_REG_ADDR(IL_TERMINAL,0))
	{
		dev->dtp.command = command;
	}
	else 
	{
		//Identificazione del subdevice nel caso la SYSCALL sia stata effettuata da un terminale
		if(subdevice)
		{
			dev->term.recv_command = command;
			key =(int*) dev->term.recv_status;
		}
		else
		{
			dev->term.transm_command = command;
			key = (int*) dev->term.transm_status;
		}
	}
	//Inserimento del processo corrente in coda su di un semaforo 
	insertBlocked(key, currentProcess);
	schedule();
}
