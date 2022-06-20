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
	next->prev = new_lst;
	new_lst->next = next;
	new_lst->prev = prev;
	prev->next = new_lst;
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
