#ifndef SHARED_VARIABLE_H
#define SHARED_VARIABLE_H

#include "queue.h"
#include "timer.h"
#define UART_BUF_MAX_SIZE 1024
struct queue read_buf, write_buf;
timer_event timers[200];
unsigned int timer_cnt;
unsigned int timer_enable; // 1 means have been enable

void init_shared_variables();

#endif
