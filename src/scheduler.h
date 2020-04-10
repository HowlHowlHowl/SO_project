#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "pcb.h"

void initScheduler(void);
void addProcess(pcb_t* p, int priority);
void setIdleProcess(pcb_t* p);
void updateCurrentProcess(state_t* state);
void terminateCurrentProcess(void);
void schedule(void);

#endif