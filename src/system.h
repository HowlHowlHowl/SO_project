#ifndef SYSTEM_H
#define SYSTEM_H

#ifdef TARGET_UMPS

#include "umps/libumps.h"
#include "umps/arch.h"
#include "umps/types.h"
#include "umps/cp0.h"

//Definizioni per compatibilita' con macro di UARM
#define CAUSE_IP_GET(cause, n) ((cause) & (CAUSE_IP(n)))

#define SYSBK_NEWAREA   0x200003D4
#define SYSBK_OLDAREA   0x20000348
#define PGMTRAP_NEWAREA 0x200002BC
#define PGMTRAP_OLDAREA 0x20000230
#define TLB_NEWAREA     0x200001A4
#define TLB_OLDAREA     0x20000118
#define INT_NEWAREA     0x2000008C
#define INT_OLDAREA     0x20000000

#define FRAME_SIZE      4096

#define DEV_TRCV_S_CHARRECV 5
#define DEV_TTRS_S_CHARTRSM 5

#define getTODLO() (*(unsigned int *)BUS_REG_TOD_LO)

//Macro per la gestione dei registri dello stato in modo analogo per entrambe le architetture
#define STATE_EXCCODE(s) CAUSE_GET_EXCCODE((s)->cause)
#define STATE_SYSCALL_NUMBER(s) (s)->reg_a0
#define STATE_SYSCALL_P1(s) (s)->reg_a1
#define STATE_SYSCALL_P2(s) (s)->reg_a2
#define STATE_SYSCALL_P3(s) (s)->reg_a3
#define STATE_CAUSE(s) (s)->cause
#define STATE_SYSCALL_RETURN(s) (s)->reg_v0

#define STATUS(s) (s)->status
#define PC(s) (s)->pc_epc
#define SP(s) (s)->reg_sp

#endif



#ifdef TARGET_UARM

#include "uarm/libuarm.h"
#include "uarm/arch.h"
#include "uarm/uARMtypes.h"
#include "uarm/uARMconst.h"

//Macro per la gestione dei registri dello stato in modo analogo per entrambe le architetture
#define STATE_EXCCODE(s) CAUSE_EXCCODE_GET((s)->CP15_Cause)
#define STATE_SYSCALL_NUMBER(s) (s)->a1
#define STATE_SYSCALL_P1(s) (s)->a2
#define STATE_SYSCALL_P2(s) (s)->a3
#define STATE_SYSCALL_P3(s) (s)->a4
#define STATE_SYSCALL_RETURN(s) (s)->a1
#define STATE_CAUSE(s) (s)->CP15_Cause

#define STATUS(s) (s)->cpsr
#define PC(s) (s)->pc
#define SP(s) (s)->sp

#endif

#define RAMBASE    *((unsigned int *)BUS_REG_RAM_BASE)
#define RAMSIZE    *((unsigned int *)BUS_REG_RAM_SIZE)
#define RAMTOP     (RAMBASE + RAMSIZE)

#endif