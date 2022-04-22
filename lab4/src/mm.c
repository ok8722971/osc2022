#include "mm.h"
#include "mini_uart.h"

#define PAGE_SIZE 4096
#define MAX_BUDDY_ORDER 9
#define PAGE_NUM (0x10000000/PAGE_SIZE) 

struct buddy_t buddies[MAX_BUDDY_ORDER];
struct page_t pages[PAGE_NUM];
struct pool_t pools[5];
int pool_size[5] = {16, 64, 256, 1024, 2048};

// from linker
extern unsigned char __mm_start;
extern unsigned char __mm_end;

// global variable
unsigned char *page_start_address;
unsigned char *page_end_address;

void init_buddy() {
	for(int i = 0; i < MAX_BUDDY_ORDER; ++i) {
		init_list_head(&(buddies[i].head));
		buddies[i].nr_free = 0;
	}
}

void init_page() {
	int order = MAX_BUDDY_ORDER - 1;
	int counter = 0, i = 0;
	while(i < PAGE_NUM){
		if(counter) {
			pages[i].used = AVAL;
			pages[i].order = -1;
			pages[i].idx = i;
			pages[i].list.next = 0;
			pages[i].list.prev = 0;
			--counter;
			++i;
		}
		else if(i + (1 << order) - 1 < PAGE_NUM) {
			pages[i].used = AVAL;
			pages[i].order = order;
			pages[i].idx = i;
			buddy_push(&(buddies[order]), &(pages[i].list));
			counter = (1 << order) -1;
			++i;
		}
		else {
			// reduce order and try again
			--order;
		}
	}
}

void init_pool() {
	for(int i = 0; i < 5; ++i) {
		pools[i].size = pool_size[i];
		init_list_head(&(pools[i].free));
		unsigned char* addr = (unsigned char*)buddy_alloc(0);
		pools[i].pfn[0] = address_to_pfn(addr);
		pools[i].page_now = addr;
		pools[i].used_obj = 0;
		pools[i].max_obj = PAGE_SIZE / pools[i].size;
		pools[i].nr_page = 1;
	}
}


void init_mm() {
	page_start_address = &__mm_start;
	page_end_address = &__mm_end;
	init_buddy();
	init_page();
	init_pool();
}

void print_buddy_info() {
	for(int i = 0; i < MAX_BUDDY_ORDER; ++i) {
		uart_printf("i:%d nr:%d\n", i, buddies[i].nr_free);
	}
	//uart_printf("%d\n", PAGE_NUM);
}

void buddy_push(struct buddy_t *bd, struct list_head *elmt) {
	bd->nr_free++;
	list_add_tail(elmt, &bd->head);
}

void buddy_remove(struct buddy_t *bd, struct list_head *elmt) {
	if(bd->nr_free == 0 || (elmt->next == 0 && elmt->prev == 0 )) return;
	bd->nr_free--;
	list_del(elmt);
}

void *buddy_alloc(int order) {
	for(int i = order; i < MAX_BUDDY_ORDER; ++i) {
		if(buddies[i].nr_free > 0) {
			// first buddy's address is the return address;
			struct list_head *first = buddies[i].head.next;
			
			// if order bigger then target then split it
			buddy_split(order, i);
			struct page_t *page = (struct page_t*)first;
			void *address = page_start_address + (page->idx)*PAGE_SIZE;
			
			// debug
			uart_printf("address=%x, page:%d\n", address, page->idx);
				
			return address;
		}
	}

	uart_printf("out of mem.\n");
	return 0;
}

void buddy_split(int target_order, int aval_order) {
	struct list_head *aval_list = buddies[aval_order].head.next;
	struct page_t *aval_page = (struct page_t*)aval_list;
	
	// debug
	if(buddies[aval_order].nr_free <= 0) {
		uart_printf_sync("some thing wrong in buddy_split\n");
	}
	
	// for demo
	uart_printf("before split:\n");
	print_buddy_info();
	
	buddy_remove(&buddies[aval_order], aval_list);
	//buddies[aval_order].nr_free--;
	//list_del(aval_list);
	aval_page->used = USED;

	// need to split
	if(target_order != aval_order) {
		int idx = aval_page->idx;
		for(int i = aval_order-1; i >= target_order; --i) {
			pages[idx + (1 << i)].order = i;
			buddy_push(&(buddies[i]), &(pages[idx + (1 << i)].list));
		}
		aval_page->order = target_order;
	}

	// for demo
	uart_printf("after split:\n");
	print_buddy_info();
}

void buddy_free(void *addr) {

	// for demo
	uart_printf("before free:\n");
	print_buddy_info();

	
	struct page_t *page = address_to_page(addr);
	struct page_t *buddy = find_buddy(page->idx, page->order);
	//uart_printf("page idx:%d, buddy idx:%d\n", page->idx, buddy->idx);
	page->used = AVAL;

	while(page->order < MAX_BUDDY_ORDER - 1 && 
		  buddy->used == AVAL &&
		  buddy->order == page->order) {

		if(page < buddy) {
			buddy_remove(&buddies[buddy->order], &(buddy->list));
			buddy->order = -1;
			page->order++;
		}
		else if (page > buddy){
			// if the page is buddy from former while loop
			// then it needs to be free too.
			buddy_remove(&buddies[page->order], &(page->list));
			page->order = -1;	
			page = buddy;
			page->order++;
		}
		else {
			// debug
			uart_printf_sync("something wrong in buddy_free\n");
		}
		buddy = find_buddy(page->idx, page->order);
	}
	
	buddy_push(&(buddies[page->order]), &(pages[page->idx].list));
	
	// for demo
	uart_printf("after free:\n");
	print_buddy_info();

	/*for(int i = 0; i < PAGE_NUM; ++i) {
		if(pages[i].used == USED) uart_printf("used:%d\n", i);
	}*/
}

struct page_t* find_buddy(int pfn, int order) {
	int buddy_pfn = pfn ^ (1 << order);
	return &pages[buddy_pfn];
}

struct page_t* address_to_page(void *address) {
	int idx = (address - (void*)page_start_address) / PAGE_SIZE;
	return &pages[idx];
}

int address_to_pfn(void *address) {
	int idx = (address - (void*)page_start_address) / PAGE_SIZE;
	return idx;
}

void *obj_alloc(struct pool_t *pool) {
    // reused free obj
    if(!list_empty(&(pool->free))) {
        struct list_head *obj = pool->free.next;
        list_del(obj);
        return obj;
    }

    // need new page
    if (pool->used_obj == pool->max_obj) {
    	struct page_t *page = (struct page_t*)buddy_alloc(0);
		pool->pfn[pool->nr_page] = page->idx;
		pool->page_now = (unsigned char*)page;
		pool->used_obj = 0;
		pool->nr_page++;
	}

    // allocate new obj
    unsigned char *addr = pool->page_now + pool->used_obj * pool->size;
    pool->used_obj++;
	
	// demo
	uart_printf("alloc addr:%x\n", addr);
	uart_printf("obj alloc size:%d, used_obj:%d\n", pool->size, pool->used_obj);

    return (void*) addr;
}

void obj_free(struct pool_t *pool, void* addr) {
    list_add_tail((struct list_head*)addr, &(pool->free));
}

void *kmalloc(unsigned long long size) {
    if (size > pool_size[4]) {
        uart_printf("kmalloc using buddy\n");
        int order;
        for (int i = 0; i < MAX_BUDDY_ORDER; i++) {
            if (size <= (unsigned long long)((1 << i) * PAGE_SIZE)) {
                order = i;
                break;
            }
        }
        return buddy_alloc(order);
    }
    else {
        uart_printf("kmalloc using object allocator\n");
        for (int i = 0; i < 5; i++) {
            if (pools[i].size >= size) {
                return (void*) obj_alloc(&pools[i]);
            }
        }
    }

	uart_printf("something wrong in kmalloc'n");
	return 0;
}

void kfree(void* addr) {
	int pfn = address_to_pfn(addr);
    for (int i = 0; i < 5; i++) {
        struct pool_t *pool = &pools[i];
        for (int j = 0; j < pool->nr_page; j++) {
        	if(pool->pfn[j] == pfn) {
				uart_printf("free using obj\n");
				obj_free(pool, addr);
				return;
			}
		}
    }
    uart_printf("free using buddy\n");
    buddy_free(addr);
}
