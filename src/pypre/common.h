#ifndef _COMMON_H_
#define _COMMON_H_ 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "config.h"


#define LOG_ERROR(op, message) op "::" message "\n"
#define LOG_ERRNO(op) LOG_ERROR(op, "%s"), strerror(errno)
#define LOG_WARN(message) "warning::" message "\n"

#define eprintf(...) fprintf(stderr, __VA_ARGS__)

#define warn(...) do { \
    if(!global_config.suppress_warns) \
        eprintf(__VA_ARGS__); } while(0)

#define die(...) do { \
    eprintf(__VA_ARGS__); \
        exit(EXIT_FAILURE);} while(0)


extern char *progname;


//extern void die(const char *, ...);
extern void *safe_malloc(size_t);
extern void *safe_calloc(size_t, size_t);
extern void *safe_realloc(void *, size_t);
extern void *safe_mmap(void *, size_t, int, int, int, off_t);

#endif
