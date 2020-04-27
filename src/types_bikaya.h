#ifndef TYPES_BIKAYA_H_INCLUDED
#define TYPES_BIKAYA_H_INCLUDED

#ifdef TARGET_UMPS
#include "umps/libumps.h"
#include "umps/arch.h"
#include "umps/types.h"
#endif
#ifdef TARGET_UARM
#include "uarm/libuarm.h"
#include "uarm/arch.h"
#include "uarm/uARMtypes.h"
#include "uarm/uARMconst.h"
#endif

#include "const.h"
#include "listx.h"

typedef unsigned int memaddr;

/*Struttura per il controllo della old e new area degli handler per ogni processo*/
typedef struct handler_areas {
    state_t* old_area;
    state_t* new_area;
} handler_areas;

enum {
    SPECPASSUP_TYPE_SYSBK   = 0,
    SPECPASSUP_TYPE_TLB     = 1,
    SPECPASSUP_TYPE_PGMTRAP = 2,
    
    SPECPASSUP_NUM_TYPES    = 3,
};

/* Process Control Block (PCB) data structure */
typedef struct pcb_t {
    /*process queue fields */
    struct list_head p_next;

    /*process tree fields */
    struct pcb_t *   p_parent;
    struct list_head p_child, p_sib;

    /* processor state, etc */
    state_t p_s;

    /* process priority */
    int priority;
    int original_priority;

    /* key of the semaphore on which the process is eventually blocked */
    int *p_semkey;
    
    /* Areas assigned by the SPECPASSUP syscall */
    handler_areas specpassup_areas[SPECPASSUP_NUM_TYPES];

    /* time record*/
    unsigned int begin_timestamp;
    unsigned int user_time;
    unsigned int kernel_time;
} pcb_t;



/* Semaphore Descriptor (SEMD) data structure */
typedef struct semd_t {
    struct list_head s_next;

    // Semaphore key
    int *s_key;

    // Queue of PCBs blocked on the semaphore
    struct list_head s_procQ;
} semd_t;

typedef struct semdev {
    semd_t disk[DEV_PER_INT];
    semd_t tape[DEV_PER_INT];
    semd_t network[DEV_PER_INT];
    semd_t printer[DEV_PER_INT];
    semd_t terminalR[DEV_PER_INT];
    semd_t terminalT[DEV_PER_INT];
} semdev;

#endif
