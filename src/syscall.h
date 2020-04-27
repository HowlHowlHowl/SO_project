#ifndef SYSCALL_H
#define SYSCALL_H

void syscallGetCPUTime(unsigned int* user, unsigned int* kernel, unsigned int *wallclock);
int  syscallCreateProcess(state_t* statep, int priority, void **cpid);
int  syscallTerminateProcess(void* pid);
void syscallVerhogen(int* semaddr);
void syscallPasseren(int* semaddr);
void syscallDo_IO(unsigned int command, unsigned int *reg, int subdevice);
int  syscallSpecPassup(int type, state_t* old, state_t* new_);
void syscallGetPidPPid(void** pid, void** ppid);

#endif
