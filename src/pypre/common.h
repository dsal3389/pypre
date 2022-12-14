#ifndef _COMMON_H_
#define _COMMON_H_ 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "config.h"


#define LOG_FORMAT(prefix, type, message) prefix ": " type ": " message "\n"
#define LOG_ERROR(prefix, message) LOG_FORMAT(prefix, "error", message)
#define LOG_ERRNO(prefix) LOG_ERROR(prefix, "%s"), strerror(errno)
#define LOG_WARN(prefix, message) LOG_FORMAT(prefix, "warning", message)
#define LOG_NOTE(prefix, message) LOG_FORMAT(prefix, "note", message)

#define eprintf(...) fprintf(stderr, __VA_ARGS__)

#define warn(...) do { \
    if(!global_config.suppress_warns) \
        eprintf(__VA_ARGS__); } while(0)

#define die(...) do { \
    eprintf(__VA_ARGS__); \
        exit(EXIT_FAILURE);} while(0)

#define note(...) printf(__VA_ARGS__)


extern char *progname;


extern void *safe_malloc(size_t);
extern void *safe_calloc(size_t, size_t);
extern void *safe_realloc(void *, size_t);
extern void *safe_mmap(void *, size_t, int, int, int, off_t);

#endif
