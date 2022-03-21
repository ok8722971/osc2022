#include "mini_uart.h"
#include "string.h"
#include "ramdisk.h" 
#include "shell.h"
#include "alloc.h"
#include "dtb.h"

#define PM_PASSWORD     0x5a000000
#define PM_RSTC         ((volatile unsigned int*)0x3F10001c)
#define PM_WDOG         ((volatile unsigned int*)0x3F100024)

void error_buffer_overflow(char *cmd) {
	uart_puts("command: \"");
	uart_puts(cmd);
	uart_puts("\" is too long, plz make sure cmd length not longer then 64\n");	
}

void error_cmd_not_found(char *cmd) {
	uart_puts("command: \"");
	uart_puts(cmd);
	uart_puts("\" is not found, plz check \"help\"\n");
}

void cmd_help(){
    uart_puts("\n");
    uart_puts("help\t: print this help menu\n");
    uart_puts("hello\t: print Hello World!\n");
    uart_puts("reboot\t: reboot the device\n");
    uart_puts("ls\t: list all file name\n");
	uart_puts("cat\t: input file name and get its content\n");
	uart_puts("test malloc\t: test simple malloc function\n");
	uart_puts("test\t: test some function\n");
	uart_puts("\n");
}

void cmd_hello(){
    uart_puts("Hello World!\n");
}

void cmd_reboot(){
	uart_puts("Start Rebooting...\n\n");

    *PM_WDOG = PM_PASSWORD | 0x20;
    *PM_RSTC = PM_PASSWORD | 100;
    
	while(1){} // hang until rebooting
}

void cmd_ls(){
	cpio_ls();
}

void cmd_cat(){
	char filename[128];
	uart_puts("Filename: ");
	normal_input(filename);
	cpio_cat(filename);
}

void cmd_test_malloc() {
	char *string = simple_malloc(8);
	char str[128];
	long long l = (long long) string;
	ltoxstr(l, str);
	uart_puts(str);
	uart_puts("\n");
	*string = 'a';
	*(string+1) = 'b';
	*(string+2) = '\0';
	uart_puts(string);
	uart_puts("\n");
}

void cmd_test(){
	dtb_get_initrd_address();
}
