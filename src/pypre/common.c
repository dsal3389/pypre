#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <sys/mman.h>

#include "common.h"


char *progname;


/* 
validate that the given pointer is not NULL, if it is NULL
then the program will die and print an error message
*/
static inline void * __validate_heap_pointer(void *ptr)
{
    if(ptr == NULL)
        die(LOG_ERRNO("heap-allocation"));
    return ptr;
}

void *safe_malloc(size_t size)
{
    return __validate_heap_pointer(malloc(size));
}

void *safe_calloc(size_t nmembers, size_t size)
{
    return __validate_heap_pointer(calloc(nmembers, size));
}

void *safe_realloc(void *ptr, size_t size)
{
    return __validate_heap_pointer(realloc(ptr, size));
}

void *safe_mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset)
{
    return __validate_heap_pointer(mmap(addr, length, prot, flags, fd, offset));
}
