#ifndef UTILS_H
#define UTILS_H

void zero_memory(void* ptr, unsigned int size);
void copy_memory(void* dest, void* source, unsigned int size);

//Macro per il massimo tra due numeri
#define MAX(a, b) ((a) > (b) ? (a) : (b))

#endif

