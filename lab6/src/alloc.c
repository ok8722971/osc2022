#include "alloc.h"
#include "string.h"
#include "mini_uart.h"

extern unsigned char __end;

void *free_ptr = (unsigned char*)(&__end);


void *simple_malloc(unsigned int size) {
	/*
	char str[128];
	long long l = (long long) &__begin;
	ltoxstr(l, str);
	uart_puts(str);
	uart_puts("\n");

	l = (long long) &__end;
    ltoxstr(l, str);
    uart_puts(str);
	uart_puts("\n");

	l = (long long) (&__end - &__begin);	
    ltoxstr(l, str);
    uart_puts(str);
	uart_puts("\n");
	*/

	free_ptr += size;
	return free_ptr - size;
}
