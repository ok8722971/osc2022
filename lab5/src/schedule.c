#include "schedule.h"
#include "queue.h"
#include "mini_uart.h"
#include "mm.h"
#include "sys.h"
#include "utils.h"
#include "ramdisk.h"

struct task_t task_pool[TASK_POOL_SIZE];
struct task_queue_t runqueue;

extern void core_timer_enable();

void init_task() {
	for(int i = 1; i < TASK_POOL_SIZE; ++i){
		task_pool[i].id = i;
		task_pool[i].status = EXIT;
		task_pool[i].reschedule = 0;
	}
	// first task, idle task
	task_pool[0].id = 0;
	task_pool[0].status = RUNNING;
	task_pool[0].reschedule = 0;
	
	update_current_task(&task_pool[0]);
}

int privilege_task_create(void (*func)()) {
	
	//lock();
	//uart_printf_sync("ptc l\n");
	struct task_t *new_task;
	for (int i = 0; i < TASK_POOL_SIZE; ++i) {
		if(task_pool[i].status == EXIT) {
			new_task = &task_pool[i];
			break;
		}
	}
	//uart_printf_sync("ptc ul\n");
	new_task->status = RUNNING;
	new_task->counter = TASK_EPOCH;
	new_task->reschedule = 0;
	new_task->cpu_context.lr = (uint64_t)func;
	// kernel stack
	// sp and fp are same at the first
	// sp will go down so we plus the size of the page
	char *addr = kmalloc(PAGE_SIZE);
	new_task->kstack_alloc = addr;
	new_task->cpu_context.fp = (uint64_t)(addr+PAGE_SIZE-16); // need 16 alignment
	new_task->cpu_context.sp = new_task->cpu_context.fp;

	// user stack
	addr = kmalloc(PAGE_SIZE);
	new_task->ustack_alloc = addr;
	new_task->ufp = (uint64_t)(addr+PAGE_SIZE-16); // need 16 alignment
	new_task->usp = new_task->ufp;

	task_queue_push(&runqueue, new_task);
	//unlock();
	return new_task->id;
}

void context_switch(struct task_t* next) {
	//uart_printf("cw start~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
	struct task_t *prev = get_current_task();
	//uart_printf("switching from %d to %d\n", prev->id, next->id);
	// not finish yet
    if (prev->status == RUNNING) {
        task_queue_push(&runqueue, prev);
    }
    update_current_task(next);
    switch_to(&prev->cpu_context, &next->cpu_context);
	//uart_printf("cw end~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
	//uart_printf_sync("sche ul\n");
}

void schedule() {
	//lock();
	//uart_printf_sync("sche l\n");
	struct task_t *next = task_queue_pop(&runqueue);
	/*if(next == &task_pool[0]) {
		next = task_queue_pop(&runqueue);
	}*/
	if(next == 0) {
		//uart_printf("idle\n");
		next = &task_pool[0];
	}
	context_switch(next);
	//unlock();
}

void foo(){
	struct task_t *t = get_current_task();
	uart_printf("foo_start\n");
    for(int i = 0; i < 3; ++i) {
        uart_printf("Thread id: %d %d\n", t->id, i);
		delay(100000000);
        //schedule();
    }
	
	uart_printf("end\n");
	t->status = EXIT;
	//schedule();
}

void demo_user_program() {
	//cpio_exec("syscall.img");
	do_exec("syscall.img");

}

void init_schedule() {
    init_task_queue(&runqueue);
	
    core_timer_enable();
	/*for(int i = 0; i < 3; ++i) { // N should > 2
        privilege_task_create(foo);
    }*/
	privilege_task_create(zombie_reaper);
	privilege_task_create(demo_user_program);
	
}

void do_exec(char *fname) {
	struct task_t *current = get_current_task();
	
	char *from_addr = cpio_get_addr(fname);
	unsigned long long fs = cpio_get_fsize(fname);
	char *to_addr = kmalloc(fs);

	transfer_prog(from_addr, to_addr, fs);

	asm volatile("msr sp_el0, %0" : : "r"(current->ustack_alloc+PAGE_SIZE-16));
    asm volatile("msr elr_el1, %0": : "r"(to_addr));
	uint64_t spsr_el1 = 0; // open interrupt and jump to el0
    asm volatile("msr spsr_el1, %0" : : "r"(spsr_el1));
    asm volatile("eret");
}

void do_exit(int status) {
	//lock();
	struct task_t *current = get_current_task();
    current->status = ZOMBIE;
    current->exit_status = status;
	
	
    // WARNING: release user stack if dynamic allocation
	kfree(current->ustack_alloc);
    //unlock();
	schedule();
}

void zombie_reaper() {
    while (1) {
		//lock();
		//uart_printf_sync("zr l\n");
        for (int i = 0; i < TASK_POOL_SIZE; i++) {
            if (task_pool[i].status == ZOMBIE) {
				uart_printf("reaper %d!\n", i);
                task_pool[i].status = EXIT;
                // WARNING: release kernel stack if dynamic allocation
				kfree(task_pool[i].kstack_alloc);
			}
        }
		//unlock();
		//uart_printf_sync("zr ul\n");
		//uart_printf("doing reaper thread\n");
        schedule();
    }
}
