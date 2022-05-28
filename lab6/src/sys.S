#include "sys_v.h"

.global s_get_pid
s_get_pid:
    mov x8, SYS_GET_PID
    svc #0
    ret

.global s_uart_read
s_uart_read:
    mov x8, SYS_UART_READ
    svc #0
    ret

.global s_uart_write
s_uart_write:
    mov x8, SYS_UART_WRITE
    svc #0
    ret

.global s_exec
s_exec:
    mov x8, SYS_EXEC
    svc #0
    ret

.global s_fork
s_fork:
    mov x8, SYS_FORK
    svc #0
    ret

.global s_exit
s_exit:
    mov x8, SYS_EXIT
    svc #0
	
.global s_mbox_call
s_mbox_call:
	mov x8, SYS_MBOX_CALL
	svc #0
	ret

.global s_kill
s_kill:
	mov x8, SYS_KILL
	svc #0
	ret
