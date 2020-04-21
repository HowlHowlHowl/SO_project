#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "pcb.h"

void initScheduler(void);
void resumeProcess(pcb_t* p);
void addProcess(pcb_t* p, int priority);
void terminateProcess(pcb_t* p);
void setIdleProcess(pcb_t* p);
void updateCurrentProcess(state_t* state);
void terminateCurrentProcess(void);
void schedule(void);
pcb_t *removeCurrentProcess(void);
pcb_t *getCurrentProcess(void);
unsigned int getTime(void);
unsigned int getTimeSliceBegin(void);
void updateTimeSliceBegin(void);

#endif
