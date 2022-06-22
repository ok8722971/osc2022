#ifndef LIST_H
#define LIST_H

#include "type.h"

#define offsetof(TYPE, MEMBER) ((uint64_t)&((TYPE *)0)->MEMBER)

#define container_of(ptr, type, member) ({                      \
        const typeof( ((type *)0)->member ) *__mptr = (ptr);    \
               (type *)( (char *)__mptr - offsetof(type,member) ); })

#define list_entry(ptr, type, member) container_of(ptr, type, member)

#define list_for_each(pos, head) \
        for (pos = (head)->next; pos != (head); pos = pos->next)

struct list_head {
	struct list_head *next, *prev;
};

void init_list_head(struct list_head *head);
int list_empty(const struct list_head *head);
void list_add(struct list_head *new_lst,
              struct list_head *head);
void list_add_tail(struct list_head *new_lst,
				   struct list_head *head);
void list_del(struct list_head *entry);

#endif
