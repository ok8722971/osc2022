#ifndef IRQ_H
#define IRQ_H

#include "mmio.h"

#define IRQ_BASE			(MMIO_BASE + 0xb000)

#define XSTR(s) str(s)
#define str(s) #s
#define IRQ_BASIC_PENDING	((unsigned int*)(IRQ_BASE + 0x200))
#define IRQ_PENDING1		((unsigned int*)(IRQ_BASE + 0x204))
#define IRQ_PENDING2		((unsigned int*)(IRQ_BASE + 0x208))
#define CORE0_INTR_SRC		((unsigned int*)0x40000060)
#define CORE0_TIMER_IRQ_CTRL	0x40000040
#define IRQ_S1 				((volatile unsigned int *)(IRQ_BASE + 0x210))

#endif
