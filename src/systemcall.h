void sysCallGetCPUTime(unsigned int* user, unsigned int* kernel, unsigned int *wallclock);
void sysCallIO(unsigned int command, unsigned int *reg, int subdevice);
void sysCallTerminate();
