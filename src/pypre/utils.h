#ifndef _UTILS_H_
#define _UTILS_H_ 1 

#include <stdlib.h>


#define LINKED_LIST_INIT {.count=0, .head=NULL, .tail=NULL}


typedef struct _link_entry{
    struct _link_entry *prev;
    struct _link_entry *next;
    unsigned long value_size;  // value byte size
    void *value;
} link_entry;

typedef struct {
    unsigned long count;
    link_entry *head;
    link_entry *tail;
} linked_list;


extern void add_entry(linked_list *, void *, size_t);
extern void delete_entry(linked_list *, link_entry *);
extern void free_list_entries(linked_list *);


#endif
