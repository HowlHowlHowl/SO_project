#ifndef SYSCALL_H
#define SYSCALL_H

void sysCallGetCPUTime(unsigned int* user, unsigned int* kernel, unsigned int *wallclock);
int sysCallCreateProcess(state_t* statep, int priority, void **cpid);
int sysCallTerminateProcess(void* pid,int a, int b);
void sysCallVerhogen(int* semaddr, int a, int b);
void sysCallPasseren(int* semaddr, int a, int b);
void sysCallIO(unsigned int command, unsigned int *reg, int subdevice);

#endif
