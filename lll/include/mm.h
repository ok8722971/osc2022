#ifndef MM_H
#define MM_H


#define MAX_POOL_SIZE 100
#define PAGE_SIZE 4096
#define MAX_BUDDY_ORDER 9
#define PAGE_NUM (0x10000000/PAGE_SIZE) 
#define KERNEL_VIRT_BASE	0xFFFF000000000000
#define MAX_USER_STACK_PAGE 4
#define PAGE_MASK	~0xFFF
#define USTACK_ADDR (0x0000fffffffff000)

#ifndef __ASSEMBLY__

#include "list.h"
#include "type.h"

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
uint64_t virtual_to_physical(uint64_t virt_addr);
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
uint64_t create_user_pgd(uint64_t filesize);
void create_user_stack(uint64_t *pgd, uint64_t start_addr, int stk_size);
void create_child_user_stack(struct task_t *current, struct task_t *child);

#endif

#endif
