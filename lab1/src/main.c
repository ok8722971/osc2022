#include "shell.h"
//#include "mini_uart.h"
//#include "string.h"

#define CMD_LEN 128

enum shell_status {
	Read,
	Parse,
};

int main() {
	shell_init();
	
	/*while(1){
		char str[256];
		itoa((int)uart_read(), str, 0);
		uart_puts(str);
	}*/

	enum shell_status status = Read;
	while(1) {
		char cmd[CMD_LEN];
		switch(status) {
			// Read cmd
			case Read:
				shell_input(cmd);
				status = Parse;
				break;

			// Parse cmd
			case Parse:
				shell_controller(cmd);
				status = Read;
				break;
		}
	}
	
	return 0;
}
