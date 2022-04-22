#ifndef MM_H
#define MM_H

#include "list.h"

#define MAX_POOL_SIZE 100

enum booking_status {
	AVAL,
	USED,
};

struct buddy_t {
	unsigned int nr_free;
	struct list_head head;
};

struct page_t {
	struct list_head list; // this need to be first postion
	enum booking_status used;
	int order;
	int idx;
};

struct pool_t {
	int size;
	int pfn[MAX_POOL_SIZE];
	unsigned char *page_now;
	int nr_page;
	int used_obj;
	int max_obj;
	struct list_head free;
};

void init_mm();
void print_buddy_info();
void buddy_push(struct buddy_t *bd, struct list_head *elmt);
void buddy_remove(struct buddy_t *bd, struct list_head *elmt);
void *buddy_alloc(int order);
void init_page();
void init_buddy();
void init_pool();
void buddy_split(int target_order, int aval_order);
void buddy_free(void *addr);
int address_to_pfn(void *address);
void *obj_alloc(struct pool_t *pool);
void obj_free(struct pool_t *pool, void* addr);
void *kmalloc(unsigned long long size);
void kfree(void* addr);
struct page_t* find_buddy(int pfn, int order);
struct page_t* address_to_page(void *address);

#endif
