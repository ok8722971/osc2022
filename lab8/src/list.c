#include "list.h"

void init_list_head(struct list_head *head) {
	head->next = head;
	head->prev = head;
}

int list_empty(const struct list_head *head) {
	return head->next == head;
}

static void __list_add(struct list_head *new_lst,
                       struct list_head *prev,
                       struct list_head *next) {
	
	//uart_printf_sync("l1\n");
	next->prev = new_lst;
	//uart_printf_sync("l2\n");
	
	new_lst->next = next;
	//uart_printf_sync("l3\n");
	
	new_lst->prev = prev;
	//uart_printf_sync("l4\n");
	
	//uart_printf_sync("%x\n", prev);
	//uart_printf_sync("%x\n", new_lst);
	//uart_printf_sync("%x\n", prev->next);
	prev->next = new_lst;
	//uart_printf_sync("l5\n");
}

void list_add(struct list_head *new_lst, 
              struct list_head *head) {
	__list_add(new_lst, head, head->next);
}

void list_add_tail(struct list_head *new_lst, 
				   struct list_head *head) {
        __list_add(new_lst, head->prev, head);
}

static void __list_del(struct list_head *prev, 
                       struct list_head *next) {
	next->prev = prev;
	prev->next = next;
}

void list_del(struct list_head *entry) {
        __list_del(entry->prev,entry->next);
        entry->next = 0;
        entry->prev = 0;
}
