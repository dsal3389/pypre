#ifndef _PREPROCESSOR_H_
#define _PREPROCESSOR_H_ 1

#include <stdlib.h>

#include "utils.h"

extern void preprocess_file(struct strbuf *, struct stat *);
extern void preprocess_directory(struct strbuf *, struct stat *);
extern void preprocess_entry(struct strbuf *, struct stat *);
extern void preprocess_text(char *, size_t *);
extern void merge_continued_lines(char*, size_t*);
extern char *parse_preprocessor_line(char *);
extern void parse_string(char *);

#endif
