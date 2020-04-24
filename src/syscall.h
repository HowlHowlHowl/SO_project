#ifndef SYSCALL_H
#define SYSCALL_H

void sysCallGetCPUTime(unsigned int* user, unsigned int* kernel, unsigned int *wallclock);
int sysCallCreateProcess(state_t* statep, int priority, void **cpid);
int sysCallTerminateProcess(void* pid,int a, int b);
void sysCallVerhogen(int* semaddr, int a, int b);
void sysCallPasseren(int* semaddr, int a, int b);
void sysCallDo_IO(unsigned int command, unsigned int *reg, int subdevice);
int sysCallSpecPassup(int type, state_t* old, state_t* new_);
void sysCallGetPidPPid(void** pid, void** ppid, int a);
#endif
