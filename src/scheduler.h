#include "pcb.h"

void init_scheduler(void);
void add_process(pcb_t* p, int priority);
void schedule(void);
