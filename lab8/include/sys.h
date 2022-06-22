#ifndef SYS_H
#define SYS_H

#include "type.h"


/* Function in sys.S */

extern int32_t s_get_pid();
extern uint32_t s_uart_read(char buf[], uint32_t size);
extern uint32_t s_uart_write(const char buf[], uint32_t size);
extern int s_exec(const char *name, char *const argv[]);
extern int s_fork();
extern void s_exit(int status);
extern int s_mbox_call(unsigned char ch, unsigned int *mbox);
extern void s_kill(int pid);

#endif 
