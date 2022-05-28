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

void init_mm_struct(struct mm_struct *m) {
    m->pgd = 0;
}

void init_task() {
	for(int i = 1; i < TASK_POOL_SIZE; ++i){
		task_pool[i].id = i;
		task_pool[i].status = EXIT;
		task_pool[i].reschedule = 0;
		init_mm_struct(&task_pool[i].mm);
	}
	// first task, idle task
	task_pool[0].id = 0;
	task_pool[0].status = RUNNING;
	task_pool[0].reschedule = 0;
	
	update_current_task(&task_pool[0]);
}

int privilege_task_create(void (*func)()) {
	
	lock();
	//uart_printf_sync("ptc l\n");
	struct task_t *new_task;
	for (int i = 0; i < TASK_POOL_SIZE; ++i) {
		if(task_pool[i].status == EXIT) {
			new_task = &task_pool[i];
			break;
		}
	}
	init_mm_struct(&new_task->mm);
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

	task_queue_push(&runqueue, new_task);
	unlock();
	return new_task->id;
}

void context_switch(struct task_t *next) {
	//uart_printf("cw start~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
	struct task_t *prev = get_current_task();
	//uart_printf("switching from %d to %d\n", prev->id, next->id);
	// not finish yet
	if (prev->status == RUNNING) {
        task_queue_push(&runqueue, prev);
    }
	//uart_printf("1\n");
    update_current_task(next);
	update_pgd(next->mm.pgd);
	unlock();
    switch_to(&prev->cpu_context, &next->cpu_context);
	//uart_printf("cw end~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
	//uart_printf_sync("sche ul\n");
}

void schedule() {
	lock();
	//uart_printf_sync("sche l\n");
	struct task_t *next = task_queue_pop(&runqueue);
	/*if(next == &task_pool[0]) {
		next = task_queue_pop(&runqueue);
	}*/
	if(next == 0) {
		//uart_printf("idle\n");
		next = &task_pool[0];
	}
	unlock();
	context_switch(next);
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
	
	/*while(1){
		uart_printf("?\n");
	}*/
	do_exec("vm.img");
}

void init_schedule() {
    init_task_queue(&runqueue);
	
    core_timer_enable();
	/*for(int i = 0; i < 3; ++i) { // N should > 2
        privilege_task_create(foo);
    }*/
	//privilege_task_create(zombie_reaper);
	privilege_task_create(demo_user_program);
	//uart_printf("123\n");
}

void do_exec(char *fname) {
	struct task_t *current = get_current_task();
	
	char *from_addr = cpio_get_addr(fname);
	unsigned long long fs = cpio_get_fsize(fname);

	//char *to_addr = kmalloc(fs);
	current->mm.pgd = create_pgd();
	map_page(current->mm.pgd,
			 0,
			 ceil(fs/(float)PAGE_SIZE));
	
	map_page(current->mm.pgd, 
			 USTACK_ADDR - PAGE_SIZE*MAX_USER_STACK_PAGE,
			 MAX_USER_STACK_PAGE);

/*	map_page2(current->mm.pgd,
			  0x3C000000,
			  0x3000000/PAGE_SIZE);
*/	
	uint64_t *p;
	p = current->mm.pgd | KERNEL_VIRT_BASE;
	p = (p[0] & PAGE_MASK) | KERNEL_VIRT_BASE; // PUD
	p = (p[0] & PAGE_MASK) | KERNEL_VIRT_BASE; // PMD
	p = (p[0] & PAGE_MASK) | KERNEL_VIRT_BASE; // PTE
	for(int i = 0; i < ceil(fs/(float)PAGE_SIZE); ++i) {
		memcpy((p[i]&PAGE_MASK) | KERNEL_VIRT_BASE, 
			   from_addr+PAGE_SIZE*i, PAGE_SIZE);
	}

	asm volatile("msr sp_el0, %0" : : "r"(USTACK_ADDR));
    asm volatile("msr elr_el1, %0": : "r"(0));
	uint64_t spsr_el1 = 0; // open interrupt and jump to el0
    asm volatile("msr spsr_el1, %0" : : "r"(spsr_el1));

	update_pgd(current->mm.pgd);
	//uart_printf("hi\n");
    asm volatile("eret");
}

void do_exit(int status) {
	lock();
	struct task_t *current = get_current_task();
    current->status = ZOMBIE;
    current->exit_status = status;
	
	
    // WARNING: release user stack if dynamic allocation
	// todo: release
	//kfree(current->ustack_alloc);
    unlock();
	schedule();
}

void zombie_reaper() {
    while (1) {
		lock();
		//uart_printf_sync("zr l\n");
        for (int i = 0; i < TASK_POOL_SIZE; i++) {
            if (task_pool[i].status == ZOMBIE) {
				//uart_printf("reaper %d!\n", i);
                //task_pool[i].status = EXIT;
                // WARNING: release kernel stack if dynamic allocation
				kfree(task_pool[i].kstack_alloc);
			}
        }
		unlock();
		//uart_printf_sync("zr ul\n");
		//uart_printf("doing reaper thread\n");
        schedule();
    }
}
