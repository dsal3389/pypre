#ifndef _UTILS_H_
#define _UTILS_H_ 1 

#include <stdlib.h>


#define STRBUF_INCREASE_CHUNK_SIZE 64

#define STRBUF_INIT { .capacity=0, .length=0, .buf=NULL }
#define STRBUF_LIST_INIT { .capacity=0, .count=0, .strings=NULL }


struct strbuf{
    size_t capacity;
    size_t length;
    char *buf;
};

struct strbuf_list{
    size_t capacity;
    size_t count;
    struct strbuf **strings;
};


extern void strbuf_set(struct strbuf *, const char *);
extern void strbuf_append(struct strbuf *, const char *);
extern void strbuf_append_char(struct strbuf *, int);
extern void strbuf_replace(struct strbuf *, long, const char *);
extern void strbuf_free(struct strbuf *);

extern void strbuf_list_append(struct strbuf_list *, const char *);
extern void strbuf_list_free(struct strbuf_list *);


#endif
