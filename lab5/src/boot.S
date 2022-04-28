.section ".text.boot"

.global _start

_start:
    mov	x11, x0
	mrs x1, mpidr_el1     // get processor id
    and x1, x1, #0xFF     // get lower 8 bit
    cbz x1, master        // if id is zero then goto master

proc_hold:
    wfe
    b proc_hold           // infinite loop if id is not zero

master:
    ldr x1, =_start       // load _start address
    mov sp, x1            // move to stack pointer
    ldr x1, =__bss_start  // get start address of bss from ld file
    ldr x2, =__bss_size   // the number need to loop
    cbz x2, 2f            // if bss size is zero then go to proc_start
1:  str xzr, [x1], #8     // store 0(xzr) to address(x0) then add x0 by 8
                          // cuz str only do 8byte/operation 
    sub x2, x2, #1        // --x1
    cbnz x2, 1b           // if not zero then goto 1 backward
	
2:	bl from_el2_to_el1	  // the next instruction runs in EL1
	b  el1_start
	//b proc_start

from_el2_to_el1:
	
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
    msr elr_el2, lr // exception link register
    eret // return to EL1

el1_start:
	// set el1 stack pointer
    ldr x1, =0x60000
    mov sp, x1

	// set el0 stack pointer
	ldr x1, =0x40000
    msr sp_el0, x1

    // load exception_table to VBAR_EL1
    adr x1, exception_table
    msr vbar_el1, x1

.global proc_start
proc_start:
    bl main               // jump to main function in C
    b proc_hold           // if return then go to infinite loop
