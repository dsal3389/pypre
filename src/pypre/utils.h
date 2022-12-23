#ifndef _UTILS_H_
#define _UTILS_H_ 1 

#include <stdio.h>
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
extern void strbuf_push(struct strbuf *, const char *);
extern void strbuf_push_char(struct strbuf *, int);
extern void strbuf_append_char(struct strbuf *, int);
extern void strbuf_replace(struct strbuf *, long, const char *);
extern void strbuf_delete(struct strbuf *, size_t, size_t);
extern void strbuf_free(struct strbuf *);

extern void strbuf_list_from_file(struct strbuf_list *, FILE *);
extern void strbuf_list_append(struct strbuf_list *, const char *);
extern void strbuf_list_remove(struct strbuf_list *, size_t);
extern void strbuf_list_free(struct strbuf_list *);

extern void tokenize_string(struct strbuf_list *, struct strbuf *);
extern void create_directories(struct strbuf *);

#endif
