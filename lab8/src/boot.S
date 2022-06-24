#define __ASSEMBLY__
#include "mm.h"
#include "mmu.h"

.section ".text.boot"

.global _start

_start:
    //mov	x11, x0
	mrs x1, mpidr_el1       // get processor id
    and x1, x1, #0xFF       // get lower 8 bit
    cbz x1, 2f

1:
    wfe
    b 1b           // infinite loop if id is not zero

2:
	
	//disable MMU
    ldr     x1, =0
    msr     sctlr_el1, x1

	// make el0, el1 can use Floating point and Advanced SIMD
    ldr     x1, =(3 << 20)
    msr     CPACR_EL1, x1

	mov x1, (1 << 31) // EL1 uses aarch64
    msr hcr_el2, x1 // hypervisor configuration registor

    mov x1, 0x3c5 // 0b1111000101 EL1h (SPSel = 1) with interrupt disabled
    msr spsr_el2, x1 // saved program status register
    
	adr x1, el1_start
	msr elr_el2, x1
    eret // return to EL1

el1_start:
	// set el1 stack pointer
    ldr x1, =0x60000
    mov sp, x1

    // load exception_table to VBAR_EL1
    ldr x1, =exception_table
    msr vbar_el1, x1

	// setup tcr
    ldr     x1, =TCR_EL1_VALUE
    msr     tcr_el1, x1

    // setup mair
    ldr     x1, =MAIR_VALUE
    msr     mair_el1, x1

    // create page table
    bl      page_table_create

    // enable MMU
    ldr     x1, =SCTLR_VALUE_MMU_ENABLE
    mrs     x2, sctlr_el1
    orr     x2, x2, x1
    msr     sctlr_el1, x2
    
    ldr x1, =__bss_start  // get start address of bss from ld file
    ldr x2, =__bss_size   // the number need to loop
    cbz x2, 2f            // if bss size is zero then go to proc_start
1:  str xzr, [x1], #8     // store 0(xzr) to address(x0) then add x0 by 8
                          // cuz str only do 8byte/operation 
    sub x2, x2, #1        // --x1
    cbnz x2, 1b           // if not zero then goto 1 backward
	
2:	// mov sp to virtual address
    ldr     x1, =KERNEL_VIRT_BASE
    add     sp, sp, x1

    // indirect branch
    // bl main
	ldr     x1, =main
    br      x1

1:
    b       1b

/* ------- */

page_table_create:
    /*
     * Each page table base address:
     *  x0: PGD (1 page, 1 entry)
     *  x1: PUD (1 page, 2 entry)
     *  x2: PMD (1 page, 512 entry)
     *  x3: PTE (512 page, each with 512 entry)
     */
    ldr     x1, =pg_dir
    lsl     x1, x1, #16  // omit first 16 bit (using physical address)
    lsr     x1, x1, #16
    add     x2, x1, #PAGE_SIZE
    add     x3, x2, #PAGE_SIZE
    add     x4, x3, #PAGE_SIZE

    // setup PGD
    ldr     x5, =PGD0_ATTR
    orr     x5, x2, x5 // PUD physical address
    str     x5, [x1]

    // setup PUD1
    ldr     x5, =PUD0_ATTR
    orr     x5, x3, x5 // PMD physical address
    str     x5, [x2]

    // setup PUD2 (1GB section start from 0x40000000)
    ldr     x5, =PUD1_ATTR
    mov     x6, 0x40000000
    orr     x5, x6, x5
    str     x5, [x2, #8]

    // setup PMD (512 entry in 1 page)
    mov     x5, x4 // point to current PTE address
    mov     x6, xzr // i = 0
    mov     x7, #512
1:
    ldr     x8, =PMD0_ATTR
    orr     x8, x5, x8 // PTE physical address
    str     x8, [x3, x6, lsl #3] // (i * 8)
    add     x6, x6, #1
    add     x5, x5, #PAGE_SIZE
    cmp     x6, x7
    b.ls    1b

    // setup normal PTE (512 * 512 - 4096 = 258048 entry)
    mov     x5, xzr // physical address
    mov     x6, xzr // i = 0
    mov     x7, #258048
2:
    ldr     x8, =PTE_NORMAL_ATTR
    orr     x8, x5, x8
    str     x8, [x4, x6, lsl #3] // (i * 8)
    add     x6, x6, #1
    add     x5, x5, #PAGE_SIZE
    cmp     x6, x7
    b.ls    2b

    // setup device PTE (16MB = 4096 entry)
    add     x7, x7, #4096 // 262144
3:
    ldr     x8, =PTE_DEVICE_ATTR
    orr     x8, x5, x8
    str     x8, [x4, x6, lsl #3] // (i * 8)
    add     x6, x6, #1
    add     x5, x5, #PAGE_SIZE
    cmp     x6, x7
    b.ls    3b

	// load PGD to the buttom translation based register.
    msr     ttbr0_el1, x1 
	// load PGD to the upper translation based register.
    msr     ttbr1_el1, x1 
    
    ret
