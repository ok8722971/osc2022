#ifndef QUEUE
#define QUEUE

#include "list.h"
#include "schedule.h"
#include "type.h"

#define QUEUE_MAX_SIZE 2048

struct queue {  // circular queue
    int front;
    int rear;
    int size;
    char buf[QUEUE_MAX_SIZE];
};

struct task_queue_t {
	struct list_head list;
	struct task_t *task;
};

void init_queue(struct queue* q, int size);
int queue_empty(struct queue* q);
int queue_full(struct queue* q);
void queue_push(struct queue* q, char val);
char queue_pop(struct queue* q);

void init_task_queue(struct task_queue_t *q);
void task_queue_push(struct task_queue_t *q, struct task_t *task);
struct task_t* task_queue_pop(struct task_queue_t *q);
void task_queue_del(struct task_queue_t *q, uint64_t id);

#endif

