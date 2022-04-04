#include "shell.h"
//#include "mini_uart.h"
//#include "string.h"

#define CMD_LEN 128

int main() {
	shell_init();
	
	/*while(1){
		char str[256];
		itoa((int)uart_read(), str, 0);
		uart_puts(str);
	}*/

	while(1) {
		char cmd[CMD_LEN];
		shell_input(cmd);
		shell_controller(cmd);
	}
	
	return 0;
}
