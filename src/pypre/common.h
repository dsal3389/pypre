#ifndef _COMMON_H_
#define _COMMON_H_ 1

#include <stdlib.h>
#include <string.h>
#include <errno.h>


#define LOG_ERROR(op, message) op "::" message "\n"
#define LOG_ERRNO(op) LOG_ERROR(op, "%s"), strerror(errno)


extern char *progname;


extern void die(const char *, ...);
extern void *safe_malloc(size_t);
extern void *safe_calloc(size_t, size_t);
extern void *safe_realloc(void *, size_t);
extern void *safe_mmap(void *, size_t, int, int, int, off_t);

#endif
