#include "shell.h"
#include "schedule.h"
#include "mini_uart.h"
#include "utils.h"
#include "mm.h"
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
        for (int i = 0; i < TASK_POOL_SIZE; i++) {
            if (task_pool[i].status == ZOMBIE) {
				//uart_printf("reaper %d!\n", i);
                //task_pool[i].status = EXIT;
                // WARNING: release kernel stack if dynamic allocation
				kfree(task_pool[i].kstack_alloc);
			}
        }
		schedule();
		//uart_printf_sync("doing idle thread\n");
	}
	return 0;
}
