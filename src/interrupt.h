#ifndef INTERRUPT_H
#define INTERRUPT_H

void wakeUpProcess(int* key, unsigned int status);
int  checkDeviceInterrupts(unsigned int cause);

#endif

