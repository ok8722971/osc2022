SECTIONS
{
	. = 0xFFFF000000000000;
    . += 0x80000;
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

	. = ALIGN(0x1000);
	pg_dir = .;
	.data.pgd :
	{
		. += (515 * (1 << 12)); 
		/* (PGD * 1) + (PUD * 1) + (PMD * 1) + (PTE * 512) */
	}
	. = ALIGN(0x1000);

	__end = .;
}

__bss_size = (__bss_end - __bss_start) >> 3; /* same */
__user_program_start = 0xFFFF000001000000;
__mm_start = 0xFFFF000010000000;
__mm_end = 0xFFFF000020000000;
/*__img_size = (__end - __begin);*/
