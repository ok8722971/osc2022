#include "shared_variables.h"

void init_shared_variables() {
	timer_cnt = 0;
	timer_enable = 0;
    init_queue(&read_buf, UART_BUF_MAX_SIZE);
    init_queue(&write_buf, UART_BUF_MAX_SIZE);
}
