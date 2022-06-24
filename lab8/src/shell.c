#include "shared_variables.h"
#include "mailbox.h"
#include "string.h"
#include "mini_uart.h"
#include "command.h"
#include "dtb.h"
#include "ramdisk.h"
#include "irq.h"
#include "aux.h"
#include "mm.h"

enum ANSI_ESC {
    Unknown,
    CursorForward,
    CursorBackward,
    Delete
};

enum ANSI_ESC decode_csi_key() {
    char c = uart_read();
	// right arrow
    if (c == 'C') {
        return CursorForward;
    }
	// left errow
    else if (c == 'D') {
        return CursorBackward;
    }
    /*else if (c == '3') {
        c = uart_read();
        if (c == '~') {
            return Delete;
        }
    }*/
    return Unknown;
}


enum ANSI_ESC decode_ansi_escape() {
    char c = uart_read();
    if (c == '[') {
        return decode_csi_key();
    }
    return Unknown;
}


void init_shell() {
	init_shared_variables();
    init_uart();
	//init_dtb();
	init_cpio();
	init_mm();
	

    //Welcome Messages
	/*uart_printf("\nSuccessful boot\n");
    mbox_board_revision();
    mbox_vc_memory();
	uart_puts("\n");*/
}


void shell_controller(char *cmd) {
    if (strcmp(cmd, "")) {
        return;
    }
    else if (strcmp(cmd, "help")) {
    	cmd_help();
	}
    else if (strcmp(cmd, "hello")) {
    	cmd_hello();
	}
    else if (strcmp(cmd, "reboot")) {
        cmd_reboot();
    }
	else if (strcmp(cmd, "ls")) {
		cmd_ls();
	}
	else if (strcmp(cmd, "cat")) {
		cmd_cat();
	}
	else if (strcmp(cmd, "test malloc")) {
		cmd_test_malloc();
	}
	else if (strcmp(cmd, "exec")) {
		cmd_exec();
	}
	else if (strcmp(cmd, "timer")) {
		cmd_timer();
	}
	else if (strcmp(cmd, "test")) {
		//asm volatile("svc #0");
		//asm volatile("svc #1");
		do_exec("vm.img");
	}
	else if (strlen(cmd) > 7 && strncmp(cmd, "malloc", 6)){
		//print_buddy_info();
		char s[128] = {0};
		int j = 0;
		for(int i = 7; cmd[i] != '\0'; ++i){
			s[j++] = cmd[i];
		}
		s[j] = '\0';
		//char *str = buddy_alloc(atoi(s));
		char *str = kmalloc(atoi(s));

		str[0] = 't';
		str[1] = 'e';
		str[2] = 's';
		str[3] = 't';
		str[4] = '\0';
		uart_printf("%s\n", str);
		kfree(str);
	}
	else if (strlen(cmd) > 13 && strncmp(cmd, "setTimeout", 10)) {
		char s1[128] = {0};
		char s2[128] = {0};
		int i, j = 0;
		for(i = 11; cmd[i] != ' '; ++i) {
			s1[j++] = cmd[i];
		}
		s1[j++] = '\0';
		i += 1;
		j = 0;
		for(; cmd[i] != '\0'; ++i) {
			s2[j++] = cmd[i];
		}
		s2[j] = '\0';
		cmd_timeout(s1, atoi(s2));
	}
    else {
    	error_cmd_not_found(cmd);
	}
}

void shell_input(char *cmd) {
    uart_puts("\r# ");

    int idx = 0, end = 0, i;
    cmd[0] = '\0';
    char c;
    while ((c = uart_read()) != '\n') {
        // Decode CSI key sequences
		/*
		By pressing one arrow key getch will push three values into the buffer:

		'\033'
		'['
		'A', 'B', 'C' or 'D'
		*/
        if (c == 27) {
            enum ANSI_ESC key = decode_ansi_escape();
            switch (key) {
                case CursorForward:
                    if (idx < end) idx++;
                    break;

                case CursorBackward:
                    if (idx > 0) idx--;
                    break;

                case Delete:
                    // left shift command
                    for (i = idx; i < end; i++) {
                        cmd[i] = cmd[i + 1];
                    }
                    cmd[--end] = '\0';
                    break;

                case Unknown:
                    break;
            }
        }
        // CTRL-C
        else if (c == 3) {
            cmd[0] = '\0';
            break;
        }
        // Backspace
        else if (c == 8 || c == 127) {
            if (idx > 0) {
                idx--;
                // left shift command
                for (i = idx; i < end; i++) {
                    cmd[i] = cmd[i + 1];
                }
                cmd[--end] = '\0';
            }
        }
        else {
            // right shift command
            if (idx < end) {
                for (i = end; i > idx; i--) {
                    cmd[i] = cmd[i - 1];
                }
            }
            cmd[idx++] = c;
            cmd[++end] = '\0';
        }
		//uart_puts("\n#");
		//uart_puts(cmd);
		uart_write(c);
    }

    uart_puts("\n");
}


void normal_input(char *cmd) {
    int idx = 0, end = 0, i;
    cmd[0] = '\0';
    char c;
    while ((c = uart_read()) != '\n') {
        // Decode CSI key sequences
		/*
		By pressing one arrow key getch will push three values into the buffer:

		'\033'
		'['
		'A', 'B', 'C' or 'D'
		*/
        if (c == 27) {
            enum ANSI_ESC key = decode_ansi_escape();
            switch (key) {
                case CursorForward:
                    if (idx < end) idx++;
                    break;

                case CursorBackward:
                    if (idx > 0) idx--;
                    break;

                case Delete:
                    // left shift command
                    for (i = idx; i < end; i++) {
                        cmd[i] = cmd[i + 1];
                    }
                    cmd[--end] = '\0';
                    break;

                case Unknown:
                    break;
            }
        }
        // CTRL-C
        else if (c == 3) {
            cmd[0] = '\0';
            break;
        }
        // Backspace
        else if (c == 8 || c == 127) {
            if (idx > 0) {
                idx--;
                // left shift command
                for (i = idx; i < end; i++) {
                    cmd[i] = cmd[i + 1];
                }
                cmd[--end] = '\0';
            }
        }
        else {
            // right shift command
            if (idx < end) {
                for (i = end; i > idx; i--) {
                    cmd[i] = cmd[i - 1];
                }
            }
            cmd[idx++] = c;
            cmd[++end] = '\0';
        }
		//uart_puts("\n#");
		//uart_puts(cmd);
		uart_write(c);
    }

    uart_puts("\n");
}
