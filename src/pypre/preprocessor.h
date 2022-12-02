#ifndef _PREPROCESSOR_H_
#define _PREPROCESSOR_H_ 1

#include <stdlib.h>

extern void preprocess_file(const char *, struct stat *);
extern void preprocess_directory(const char *, struct stat *);
extern void preprocess_entry(const char *, struct stat *);
extern void merge_continued_lines(const char *, char *, size_t);

#endif
