SECTIONS
{
    . = 0x80000;
	__begin = .;
    .text :
    {
        KEEP(*(.text.boot)) /* make sure to be used */
        *(.text)
    }
    .rodata :
    {
    	*(.rodata)
    	*(.rodata*)
    }
    .data :
    {
        *(.data)
        *(.data*)
    }

    .bss ALIGN(16) (NOLOAD) :
    {
        __bss_start = .; /* var for ref in boot.s */
        *(.bss)
        __bss_end = .;

    }
	__end = .;
}

__bss_size = (__bss_end - __bss_start) >> 3; /* same */
__user_program_start = 0x1000000;
__mm_start = 0x10000000;
__mm_end = 0x20000000;
/*__img_size = (__end - __begin);*/
