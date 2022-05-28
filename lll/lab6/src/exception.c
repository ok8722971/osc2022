#include "mini_uart.h"
#include "irq.h"
#include "aux.h"
#include "queue.h"
#include "shared_variables.h"
#include "exception.h"
#include "type.h"
#include "sys.h"
#include "sys_v.h"
#include "schedule.h"
#include "mailbox.h"
#include "mm.h"

extern void return_from_fork();
char intr_stack[INTR_STK_SIZE];

static unsigned long long lock_count = 0;

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
	//uart_printf_sync("tick\n");
	unsigned long long cntfrq_el0;
	// get current counter frequency
	asm volatile ("mrs %0, cntfrq_el0" : "=r" (cntfrq_el0));
	cntfrq_el0 >>= 5;
	// set next time out to freq shift right 5 bits
	asm volatile ("msr cntp_tval_el0, %0": : "r" (cntfrq_el0));

	// check current task running time
    struct task_t *current = get_current_task();
    if (--current->counter <= 0) {
        current->counter = TASK_EPOCH;
        current->reschedule = 1;
    }
	//uart_printf_sync("timer interrupt! \n");	
}

void sys_get_pid(struct trapframe *trapframe) {
    uint64_t task_id = get_current_task()->id;
    trapframe->x[0] = task_id;
}

void sys_uart_read(struct trapframe *trapframe) {
    //uart_printf_sync("r\n");
	char *buf = (char*)trapframe->x[0];
    uint32_t size = trapframe->x[1];
    for (uint32_t i = 0; i < size; i++) {
        buf[i] = uart_read();
    }
    //buf[size] = '\0';
    trapframe->x[0] = size;
}

void sys_uart_write(struct trapframe *trapframe) {
	//uart_printf_sync("w\n");
    const char *buf = (char*)trapframe->x[0];
    uint32_t size = trapframe->x[1];

    for (uint32_t i = 0; i < size; i++) {
        uart_write(buf[i]);
    }

    trapframe->x[0] = size;
}

void sys_exec(struct trapframe *trapframe) {
    const char *name = (char*)trapframe->x[0];
	char *const argv = (char**) trapframe->x[1];
	trapframe->x[0] = 0;
    do_exec(name);
}

void sys_fork(struct trapframe *trapframe) {
	//lock();

	struct task_t *parent_task = get_current_task();

    int child_id = privilege_task_create(return_from_fork);
    struct task_t *child_task = &task_pool[child_id];

    char *child_kstack = child_task->kstack_alloc + PAGE_SIZE - 16;
    char *parent_kstack = parent_task->kstack_alloc + PAGE_SIZE - 16;

    uint64_t kstack_offset = parent_kstack - (char*)trapframe;

    for (uint64_t i = 0; i < kstack_offset; i++) {
        *(child_kstack - i) = *(parent_kstack - i);
    }

    // place child's kernel stack to right place
    child_task->cpu_context.sp = (uint64_t)child_kstack - kstack_offset;

	// copy all user pages
	create_child_page_table(parent_task, child_task);

    // place child's user stack to right place
    struct trapframe *child_trapframe = (struct trapframe*)child_task->cpu_context.sp;
    child_trapframe->sp_el0 = trapframe->sp_el0;

	// fork child get 0
    child_trapframe->x[0] = 0;
	// parent get child id
    trapframe->x[0] = child_task->id;
	//unlock();
}

void sys_exit(struct trapframe *trapframe) {
    do_exit(trapframe->x[0]);
}

void sys_mbox_call(struct trapframe *trapframe) {
	
	// prevent multi-thread using mailbox
	//lock();
	//uart_printf_sync("s_mbox\n");

	unsigned char channel = trapframe->x[0];
	unsigned int *mbox = (unsigned int*)trapframe->x[1];
	
	uint64_t *pgd = (get_current_task())->mm.pgd;	

	//uart_printf("%X\n", mbox);	
	mbox = user_va_to_kernel_va(pgd, mbox);
	//uart_printf("%X\n", mbox);	
	
	int success = mbox_call(mbox, channel);
	
	/*int sz = mbox[0];
	for(int i = 0; i < sz; ++i) {
		uart_printf("%D %X\n", i, mbox[i]);
	}*/

	//unlock();
	trapframe->x[0] =  success;
}

void sys_kill(struct trapframe *trapframe) {
	// prevent killing one still running
	//lock();

	uint64_t id = trapframe->x[0];
	
	if(id >= TASK_POOL_SIZE || id < 0 || task_pool[id].status != RUNNING) {
		enable_interrupt();
		return;
	}

	struct task_t *killed_t = &task_pool[id];
	// change the state
	killed_t->status = ZOMBIE;
	// move out the run queue
	task_queue_del(&runqueue, id);
	// schedule to prevent delete itself 
	
	//unlock();
	schedule();
}

void sys_call_router(uint64_t sys_call_num, struct trapframe* trapframe) {
	enable_interrupt();
	lock_count = 0;
	switch (sys_call_num) {
        case SYS_GET_PID:
            sys_get_pid(trapframe);
            break;

        case SYS_UART_READ:
            sys_uart_read(trapframe);
            break;

        case SYS_UART_WRITE:
            sys_uart_write(trapframe);
            break;

        case SYS_EXEC:
            sys_exec(trapframe);
            break;

        case SYS_FORK:
            sys_fork(trapframe);
            break;

        case SYS_EXIT:
            sys_exit(trapframe);
            break;

		case SYS_MBOX_CALL:
			sys_mbox_call(trapframe);
			break;
		case SYS_KILL:
			sys_kill(trapframe);
			break;
    }
}


void sync_exc_router(unsigned long esr, unsigned long elr, struct trapframe *trapframe) {
	//uart_printf_sync("sr\n");
	int ec = (esr >> 26) & 0b111111;
    int iss = esr & 0x1FFFFFF;
    if (ec == 0b010101) {  // system call
        uint64_t syscall_num = trapframe->x[8];
        sys_call_router(syscall_num, trapframe);
    }
    else {
		uart_puts_sync("sync!\n");
        uart_printf_sync("Exception return address %X\n", elr);
        uart_printf_sync("Exception class (EC) %X\n", ec);
        uart_printf_sync("Instruction specific syndrome (ISS) %X\n", iss);
		while(1){}
    }
}

void irq_exc_router() {
	
	disable_interrupt();
	//uart_printf_sync("ir\n");
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

void irq_stk_switcher() {
    // Switch to interrupt stack if entry_sp in kernel stack
    register char* entry_sp;
    asm volatile("mov %0, sp": "=r"(entry_sp));
    if (!(entry_sp <= &intr_stack[4095] && entry_sp >= &intr_stack[0])) {
        asm volatile("mov sp, %0" : : "r"(&intr_stack[INTR_STK_TOP_IDX]));
    }

    irq_exc_router();

    // Restore to kernel stack if entry_sp in kernel stack
    if (!(entry_sp <= &intr_stack[4095] && entry_sp >= &intr_stack[0])) {
        asm volatile("mov sp, %0" : : "r"(entry_sp));
    }
}

void irq_return() {
   	// from context_switch.S
    struct task_t *current = get_current_task();
    // check reschedule flag
	if (current->reschedule) {
        current->counter = TASK_EPOCH;
        current->reschedule = 0;
        schedule();
    }
}

void lock() {
    disable_interrupt();
    lock_count++;
	//uart_printf_sync("after lock, lock num:%d\n", lock_count);
}

void unlock() {
    lock_count--;
    if (lock_count<0)
    {
        uart_printf("lock error !!!\r\n");
        while(1);
    }
    if (lock_count == 0){
        enable_interrupt();
	}
	//uart_printf_sync("after unlock, lock num:%d\n", lock_count);
}

int get_lock() {
	return lock_count;
}
