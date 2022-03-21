.section ".text.boot"

.global _start

_start:
    mrs x0, mpidr_el1     // get processor id
    and x0, x0, #0xFF     // get lower 8 bit
    cbz x0, master        // if id is zero then goto master

proc_hold:
    wfe
    b proc_hold           // infinite loop if id is not zero

master:
    ldr x0, =_start       // load _start address
    mov sp, x0            // move to stack pointer
    ldr x0, =__bss_start  // get start address of bss from ld file
    ldr x1, =__bss_size   // the number need to loop
    cbz x1, proc_start    // if bss size is zero then go to proc_start
1:  str xzr, [x0], #8     // store 0(xzr) to address(x0) then add x0 by 8
                          // cuz str only do 8byte/operation 
    sub x1, x1, #1        // --x1
    cbnz x1, 1b           // if not zero then goto 1 backward


proc_start:
    bl main               // jump to main function in C
    b proc_hold           // if return then go to loop
