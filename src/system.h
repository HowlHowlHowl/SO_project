#ifndef SYSTEM_H
#define SYSTEM_H

#ifdef TARGET_UMPS
#include "umps/libumps.h"
#include "umps/arch.h"
#include "umps/types.h"
#include "umps/cp0.h"

#define SYSBK_NEWAREA   0x200003D4
#define SYSBK_OLDAREA   0x20000348
#define PGMTRAP_NEWAREA 0x200002BC
#define PGMTRAP_OLDAREA 0x20000230
#define TLB_NEWAREA     0x200001A4
#define TLB_OLDAREA     0x20000118
#define INT_NEWAREA     0x2000008C
#define INT_OLDAREA     0x20000000

#define FRAME_SIZE      4096
#define EXC_SYSCALL     EXC_SYS

#endif

#ifdef TARGET_UARM
#include "uarm/libuarm.h"
#include "uarm/arch.h"
#include "uarm/uARMtypes.h"
#include "uarm/uARMconst.h"
#endif


#define RAMBASE    *((unsigned int *)BUS_REG_RAM_BASE)
#define RAMSIZE    *((unsigned int *)BUS_REG_RAM_SIZE)
#define RAMTOP     (RAMBASE + RAMSIZE)

#define SYS_TERMINATEPROCESS 3

#endif