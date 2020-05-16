#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "pcb.h"

void initScheduler(void);
void addProcess(pcb_t* p, int priority);
void setIdleProcess(pcb_t* p);
void updateCurrentProcess(state_t* state);
int  terminateProcess(pcb_t* p);
void terminateCurrentProcess(void);
void resumeProcess(pcb_t* p);
pcb_t* suspendCurrentProcess(void);
pcb_t* getCurrentProcess(void);
unsigned int getTime(void);
unsigned int getUserTimeBegin(void);
void updateUserTimeBegin(void);
void schedule(void);

#endif
