#include "shell.h"
#include "schedule.h"
#include "mini_uart.h"
#include "utils.h"
//#include "string.h"

#define CMD_LEN 128

int main() {
	//delay(1000000000);
	init_task();
	init_shell();
	init_schedule();
	
	/*while(1){
		char str[256];
		itoa((int)uart_read(), str, 0);
		uart_puts(str);
	}*/

	/*while(1) {
		char cmd[CMD_LEN];
		shell_input(cmd);
		shell_controller(cmd);
	}*/

	while(1) {
		schedule();
		//uart_printf("doing idle thread\n");
	}
	return 0;
}
