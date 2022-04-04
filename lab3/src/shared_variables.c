#include "shared_variables.h"

void init_shared_variables() {
	timer_cnt = 0;
	timer_enable = 0;
    queue_init(&read_buf, UART_BUF_MAX_SIZE);
    queue_init(&write_buf, UART_BUF_MAX_SIZE);
}
