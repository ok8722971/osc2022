#ifndef SCHEDULE_H
#define SCHEDULE_H

#include "type.h"

#define TASK_EPOCH 1
#define TASK_POOL_SIZE 64

struct cpu_context {
	// ARM calling convention
	// x0 ~ x18 can be modified by the called function
	uint64_t x19;
    uint64_t x20;
    uint64_t x21;
    uint64_t x22;
    uint64_t x23;
    uint64_t x24;
    uint64_t x25;
    uint64_t x26;
    uint64_t x27;
    uint64_t x28;
    uint64_t fp;  // x29
    uint64_t lr;  // x30
    uint64_t sp;
};

enum task_status {
	RUNNING,
	ZOMBIE,
	EXIT,
};

struct mm_struct {
    uint64_t pgd;
};

struct task_t {
	uint64_t id;
	enum task_status status;
	int counter; // for round robin
	int reschedule; // reschedule flag
	int exit_status; // for exit system call
	char *kstack_alloc;
	struct mm_struct mm;
	struct cpu_context cpu_context;
};

extern struct task_queue_t runqueue;
extern struct task_t task_pool[TASK_POOL_SIZE];
 

/* Function in schedule.S */
extern struct task_t* get_current_task();
extern void update_pgd(uint64_t pgd);
extern void update_current_task(struct task_t *task);
extern void switch_to(struct cpu_context* prev, struct cpu_context* next);

/* Function in schedule.c*/
void init_task();
int privilege_task_create(void (*func)());
void context_switch(struct task_t* next);
void schedule();
void foo();
void init_schedule();
void do_exec(char *fname);
void do_exit(int status);
void zombie_reaper();
void demo_user_program();

#endif



