#include "timer.h"
#include "string.h"
#include "utils.h"
#include "shared_variables.h"
#include "mini_uart.h"
#include "irq.h"

extern void core_timer_enable();

// struct pointer need memcpy
// cause compiler error
void swap(timer_event *a, timer_event *b){
    timer_event temp = *a;
    *a = *b;
    *b = temp;
}

int partition(timer_event *arr, int front, int end){
    timer_event pivot = arr[end];
    int i = front -1;
    for (int j = front; j < end; j++) {
        if (arr[j].ticket < pivot.ticket) {
            i++;
            swap(&arr[i], &arr[j]);
        }
    }
    i++;
    swap(&arr[i], &arr[end]);
    return i;
}

void timer_quick_sort(timer_event *arr, int front, int end){
    if (front < end) {
        int pivot = partition(arr, front, end);
        timer_quick_sort(arr, front, pivot - 1);
        timer_quick_sort(arr, pivot + 1, end);
    }
}

/*void enable_timer() {
	enable_interrupt();
    asm volatile(
		"mov x1, 1\n\t"
        "msr cntp_ctl_el0, x1\n\t" // enable timer

        "mrs x1, cntfrq_el0\n\t"
        "mov x2, 0x999\n\t"
        "mul x1, x1, x2\n\t"
        "msr cntp_tval_el0, x1\n\t" // set expired time

		"ldr x2, =" XSTR(CORE0_TIMER_IRQ_CTRL) "\n\t"
        "str w1, [x2]" // unmask timer interrupt
    );
	timer_enable = 1;
}*/

void remove_front_timer() {
	int i;
	if(timer_cnt <= 0) {
		uart_puts_sync("something wrong in timer\n");
	}
	--timer_cnt;
	for(i = 0; i < timer_cnt; ++i) {
		timers[i] = timers[i+1];
	}
}

void timer_helper(char *s, float startTime) {
	unsigned long long cntfrq_el0;
	unsigned long long cntpct_el0;
	// get current counter frequency
	asm volatile ("mrs %0, cntfrq_el0" : "=r" (cntfrq_el0));
	// read current counter
    asm volatile ("mrs %0, cntpct_el0" : "=r" (cntpct_el0));
	uart_printf_sync("start time:%f secs end time:%f secs\n%s\n", 
			startTime, (float)cntpct_el0/cntfrq_el0, s);
}

void add_timer(void *callback, char *s, unsigned int timeout) {
	if(!timer_enable) {
		//enable_timer();
		core_timer_enable();
		timer_enable = 1;
	}
	timers[timer_cnt].callback = callback;
	strcpy(s, timers[timer_cnt].s);
	// set ticket;
	timers[timer_cnt].ticket = get_tick_plus_sec(timeout);
	
	// set start time
	unsigned long long cntfrq_el0;
	unsigned long long cntpct_el0;
	// get current counter frequency
	asm volatile ("mrs %0, cntfrq_el0" : "=r" (cntfrq_el0));
	// read current counter
    asm volatile ("mrs %0, cntpct_el0" : "=r" (cntpct_el0));
	timers[timer_cnt].start_time = (float)cntpct_el0/cntfrq_el0;

	// sort
	timer_quick_sort(timers, 0, timer_cnt);

	// if ticket less then next intr then change the intr time
	// 1.get time ticket
	// 2.comapre
	// 		yes:change
	unsigned int cntp_cval_el0;
	asm volatile ("mrs %0, cntp_cval_el0\n\t"
                 : "=r"(cntp_cval_el0)); //next intr ticket
	
	if(cntp_cval_el0 > timers[timer_cnt].ticket) {
		set_core_timer_interrupt(timeout);
	}
	++timer_cnt;
}

unsigned long long get_tick_plus_sec(unsigned int second) {
    unsigned long long cntpct_el0;
	asm volatile ("mrs %0, cntpct_el0\n\t"
                   : "=r"(cntpct_el0)); //tick now

    unsigned long long cntfrq_el0;
	asm volatile("mrs %0, cntfrq_el0\n\t"
                   : "=r"(cntfrq_el0)); //tick frequency

    return (cntpct_el0 + cntfrq_el0 * second);
}

void set_core_timer_interrupt(unsigned int expired_time) {
	asm volatile(
        "mrs x1, cntfrq_el0\n\t"
        "mul x1, x1, %0\n\t"
        "msr cntp_tval_el0, x1\n\t" // set expired time
        : "=r"(expired_time));
}

void set_core_timer_interrupt_by_tick(unsigned long long tick) {
	asm volatile(
        "msr cntp_cval_el0, %0\n\t"
        : "=r"(tick));
}
