#ifndef AUX_H
#define AUX_H

#include "mmio.h"
#define AUX_BASE    (MMIO_BASE + 0x215000)

#define AUX_IRQ         ((volatile unsigned int*)(AUX_BASE + 0x00))
#define AUX_ENABLES     ((volatile unsigned int*)(AUX_BASE + 0x04))
#define AUX_MU_IO       ((volatile unsigned int*)(AUX_BASE + 0x40))
#define AUX_MU_IER      ((volatile unsigned int*)(AUX_BASE + 0x44))
#define AUX_MU_IIR      ((volatile unsigned int*)(AUX_BASE + 0x48))
#define AUX_MU_LCR      ((volatile unsigned int*)(AUX_BASE + 0x4C))
#define AUX_MU_MCR      ((volatile unsigned int*)(AUX_BASE + 0x50))
#define AUX_MU_LSR      ((volatile unsigned int*)(AUX_BASE + 0x54))
#define AUX_MU_MSR      ((volatile unsigned int*)(AUX_BASE + 0x58))
#define AUX_MU_SCRATCH  ((volatile unsigned int*)(AUX_BASE + 0x5C))
#define AUX_MU_CNTL     ((volatile unsigned int*)(AUX_BASE + 0x60))
#define AUX_MU_STAT     ((volatile unsigned int*)(AUX_BASE + 0x64))
#define AUX_MU_BAUD     ((volatile unsigned int*)(AUX_BASE + 0x68))
#define AUX_SPI0_CNTL0  ((volatile unsigned int*)(AUX_BASE + 0x80))
#define AUX_SPI0_CNTL1  ((volatile unsigned int*)(AUX_BASE + 0x84))
#define AUX_SPI0_STAT   ((volatile unsigned int*)(AUX_BASE + 0x88))
#define AUX_SPI0_IO     ((volatile unsigned int*)(AUX_BASE + 0x90))
#define AUX_SPI0_PEEK   ((volatile unsigned int*)(AUX_BASE + 0x94))
#define AUX_SPI1_CNTL0  ((volatile unsigned int*)(AUX_BASE + 0xC0))
#define AUX_SPI1_CNTL1  ((volatile unsigned int*)(AUX_BASE + 0xC4))
#define AUX_SPI1_STAT   ((volatile unsigned int*)(AUX_BASE + 0xC8))
#define AUX_SPI1_IO     ((volatile unsigned int*)(AUX_BASE + 0xD0))
#define AUX_SPI1_PEEK   ((volatile unsigned int*)(AUX_BASE + 0xD4))


#endif
