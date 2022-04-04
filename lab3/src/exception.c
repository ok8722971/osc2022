#include "mini_uart.h"
#include "irq.h"
#include "aux.h"
#include "queue.h"
#include "shared_variables.h"

void uart_int_handler() {
	disable_uart_interrupt();

	// check interrupt type
	int rx = (*AUX_MU_IIR & 0x4);
	int tx = (*AUX_MU_IIR & 0X2);
	char c;

	if (rx) { // read
		c = (char)(*AUX_MU_IO);
		queue_push(&read_buf, c);
		clear_rx_interrupt();
	}
	else if (tx) { // write
		while (!queue_empty(&write_buf)) {
			c = queue_pop(&write_buf);
        	uart_write(c);
		}
		clear_tx_interrupt();
	}
	else {
		uart_printf_sync("unknown uart interrupt\n");
		while (1) {}
	}
	enable_uart_interrupt();
}

void core_timer_int_handler() {
	//uart_printf_sync("hi");
	unsigned long long cntfrq_el0;
	unsigned long long cntpct_el0;
	// get current counter frequency
	asm volatile ("mrs %0, cntfrq_el0" : "=r" (cntfrq_el0));
	// read current counter
    asm volatile ("mrs %0, cntpct_el0" : "=r" (cntpct_el0));
	// while loop to clear time event
	while(timer_cnt > 0 && timers[0].ticket <= cntpct_el0) {
		// exec timer event's call back
		((void (*)(char *, float))timers[0].callback)(timers[0].s, timers[0].start_time);
		remove_front_timer(); 
	}
	// set next time out to 2 secs later or time_event
	unsigned long long two_second_after_ticket = get_tick_plus_sec(2);
	if(timer_cnt > 0 && timers[0].ticket < two_second_after_ticket) {
		set_core_timer_interrupt_by_tick(timers[0].ticket);
	}
	else {
		set_core_timer_interrupt(2);
	}

    uart_printf_sync("%f seconds after booting\n", (float)cntpct_el0/cntfrq_el0);
	
}


void sync_exc_router(unsigned long long spsr, unsigned long elr, unsigned long esr) {
	uart_printf_sync("spsr: %X\n", spsr);
    uart_printf_sync("elr: %x\n", elr);
    uart_printf_sync("esr: %x\n", esr);
}

void irq_exc_router() {
	disable_interrupt();

	unsigned int irq_basic_pending = *(IRQ_BASIC_PENDING);
	unsigned int core0_int_src = *(CORE0_INTR_SRC);
	//uart_printf_sync("irqp:%d\n", irq_basic_pending);
	if ((*IRQ_PENDING1) & (1<<29)) {
		uart_int_handler();
    }
	else if (core0_int_src && (1<<1)) {
		core_timer_int_handler();
    }
	else {
		uart_printf_sync("Something wrong here. IRQ b p: 0x%x\n", irq_basic_pending);
		while(1) {}
	}
	enable_interrupt();
}
