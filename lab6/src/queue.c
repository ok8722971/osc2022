#include "mini_uart.h"
#include "queue.h"
#include "list.h"
#include "mm.h"

//int tasknum = 0;

void init_queue(struct queue* q, int size) {
    q->front = 0;
    q->rear = 0;
    q->size = size;
}

int queue_empty(struct queue* q) {
    return q->front == q->rear;
}

int queue_full(struct queue* q) {
    return q->front == (q->rear + 1) % q->size;
}

void queue_push(struct queue* q, char val) {
    if (queue_full(q)) return;  // drop if full
    q->buf[q->rear] = val;
    q->rear = (q->rear + 1) % q->size;
}

char queue_pop(struct queue* q) {
    if (queue_empty(q)) return '\0';
    char elmt = q->buf[q->front];
    q->front = (q->front + 1) % q->size;
    return elmt;
}

void init_task_queue(struct task_queue_t *q) {
	init_list_head(&(q->list));
	q->task = 0;
}

void task_queue_push(struct task_queue_t *q, struct task_t *task) {
	// we dont have priority now so just push to the tail
	struct task_queue_t *new_task = kmalloc(sizeof(struct task_queue_t));
	new_task->task = task;
	list_add_tail(&new_task->list, &q->list);
	//uart_printf("task num:%d\n", ++tasknum);
}

struct task_t* task_queue_pop(struct task_queue_t *q) {
	if(list_empty(&q->list)) {
		return 0;
	}
	struct task_queue_t *first_element = q->list.next;
	struct task_t *t = first_element->task;
	list_del((struct list_head*)first_element);
	kfree(first_element);
	//uart_printf("task num:%d\n", --tasknum);
	return t;
}

void task_queue_del(struct task_queue_t *q, uint64_t id) {
	for(struct list_head *p = q->list.next; p != &q->list; p = p->next) {
		struct task_queue_t *tq = p;
		struct task_t *t = tq->task;
		if(t->id == id) {
			list_del(p);
			kfree(p);
			return;
		}
	}
	// debug
	uart_printf_sync("id not found in task_queue_del\n");
}

