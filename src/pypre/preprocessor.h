#ifndef _PREPROCESSOR_H_
#define _PREPROCESSOR_H_ 1

#include <stdio.h>
#include <stdlib.h>

#include "utils.h"


extern void preprocess_entry(struct strbuf *, struct stat *);
extern void preprocess_file(FILE *, struct strbuf *);
extern void preprocess_directory(struct strbuf *, struct stat *);
extern void preprocess_text(const char *, struct strbuf_list *);
extern void merge_continued_lines(const char *, struct strbuf_list *);
extern char *parse_preprocessor_line(char *);

#endif
