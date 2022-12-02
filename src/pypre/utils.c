#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "utils.h"


void add_entry(linked_list *lls, void *value, size_t size)
{
    link_entry *entry = (link_entry *) safe_calloc(1, sizeof(link_entry));
    entry->value_size = size;
    entry->value = safe_malloc(size);

    // copy the value to allocated memory, so if the given
    // value pointer is defined on a stack frame, and the stack frame is
    // poped, then it wont break the program
    memcpy(entry->value, value, size);
    lls->count++;
    
    // if head is NULL, then there are no items on the
    // list, define the current entry as head
    if(lls->head == NULL){
        lls->head = entry;
        lls->tail = entry;
        entry->next = NULL;
        entry->prev = NULL;
    } else {
        entry->prev = lls->tail;
        lls->tail->next = entry;
        lls->tail = entry;
    }
}

void delete_entry(linked_list *lls, link_entry *entry)
{
    if(lls->head == entry)
        lls->head = entry->next;
    if(lls->tail == entry)
        lls->tail = entry->prev;
    if(entry->next)
        entry->next->prev = entry->prev;
    if(entry->prev)
        entry->prev->next = entry->next;
}

void free_list_entries(linked_list *lls)
{
    // if lls head is NULL, it means 
    // there are no values in the list 
    if(lls->head == NULL)
        return;

    link_entry *entry = lls->head;
    link_entry *tmp = NULL;
    
    while(entry){
        // we remember the pointer to the next
        // entry, because later we gonna free the entry
        // and we wont have access to that data
        tmp = entry->next;
        
        free(entry->value);
        free(entry);
        entry = tmp;
    }

    lls->head = NULL;
    lls->tail = NULL;
    lls->count = 0;
}
