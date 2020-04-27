#ifndef INTERRUPT_H
#define INTERRUPT_H

void wakeUpProcess(int* key, unsigned int status);
void checkDeviceInterrupts(unsigned int cause);

#endif

