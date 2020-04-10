#include "utils.h"

/* Mette a 0 size byte a partire da ptr */
void zero_memory(void* ptr, unsigned int size)
{
    char* p = (char*)ptr;
    for(unsigned int i = 0; i < size; i++)
    {
        p[i] = 0;
    }
}

/* Copia size byte da src a dest */
void copy_memory(void* dest, void* source, unsigned int size)
{
    char* d = (char*)dest;
    char* s = (char*)source;
    
    for(unsigned int i = 0; i < size; i++)
    {
        d[i] = s[i];
    }
}

