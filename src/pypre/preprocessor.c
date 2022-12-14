#include <linux/limits.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>

#include "preprocessor.h"
#include "common.h"
#include "config.h"
#include "token.h"
#include "utils.h"


static char *__mmap_file(const char *path, size_t size)
{
    int fd = open(path, O_RDWR);
    char *content = NULL;

    if(fd == -1)
        die(LOG_ERRNO("open"));

    content = (char *) safe_mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
    close(fd);
    return content;
}

static void __preprocess_entry(struct strbuf *entry_name, struct stat *estat)
{
    if(estat->st_mode & S_IFREG)
        preprocess_file(entry_name, estat);
    else if(estat->st_mode & S_IFLNK)
        preprocess_file(entry_name, estat);
    else if(estat->st_mode & S_IFDIR)
        preprocess_directory(entry_name, estat);
    else
        die(LOG_ERROR("process-entry", "failed to process entry, %s, maybe unsupported filetype"), entry_name->buf);
}

/*
merge_continued_lines:
this function convert lines with break char to a single line
for example:
    #define true\
    # True

converted to 
    #define true True

at this stage, we are not checking for errors or unexpected tokens
*/
void merge_continued_lines(char *content, size_t *size)
{
    char *tmp = NULL, *cptr = content;
    size_t mmove_count = 0;
    size_t lcount = 1;  // line count, for readable error messages

BEGIN:
    while(*cptr){
        if(
            // if the current char is "\"" and the next char is also "\"
            // it meants its an escape seqance "\\"
            *cptr != global_config.line_break_char ||
            *(cptr + 1) == global_config.line_break_char
        ){
            if(*cptr == '\n')
                lcount++;
            cptr++;
            continue;
        }

        // remember where we saw the link break, this will 
        // help us later know from where to where delete chars
        tmp = cptr;
        cptr++;

        // read until new line, if there is any unexpected
        // charachter after the line break, die with an error message
        while(*cptr != '\n'){
            switch(*cptr){
                case ' ':
                case '\t':
                    cptr++;
                    break;
                default:
                    // if we have unexpected char, then its probably a line 
                    // with an escape char, for example "print('hello world\n')",
                    warn(
                        LOG_WARN(
                            "merge continued lines", 
                            "unexpected char (%c) after line break, on line %ld, ignoring"), *cptr, lcount
                    );
                    goto BEGIN;
            }
        }

        cptr++;
        lcount++;

        // if after the new line the first char is `global_config.preprocess_char`
        // then we want to include it also in when we delete
        // chars
        if(*cptr == global_config.preprocess_char)
            cptr++;


        // count how many bytes we need to move, by calculating
        // how many chars we count so far and subtract it from the 
        // current size, for example 15 - (0 - 10) = 5, + 1 for null terminator
        mmove_count = *size - (cptr - content) + 1;

        // update the size, what is the length of what we are trying
        // to delete and add it to the size, for example
        // 10 + (2 - 5) = 7
        *size += tmp - cptr;

        // delete all chars from where we found the line break
        // up to the new line, this is the actual merge
        memmove(tmp, cptr, mmove_count);
        cptr = tmp;
    }
}

void tokenize_and_parse_line(char *line)
{
    struct strbuf token = STRBUF_INIT;
    char *track = line;

    printf("processing %s\n", line);
    for(;;){
        track = get_next_word(&token, track);
        if(track == NULL)
            break;
        printf("\ttoken: %s\n", token.buf);
    }
    putchar('\n');
    strbuf_free(&token);
}

/* parsing each line on the given string */
void tokenize_and_parse_text(char *text, size_t *size)
{
    char *line, *line_buffer, *ptr;

    // we parse each line seperatly and pass it to a function
    // that will tokenize and parse each word in the line
    for(ptr=text; ; ptr=NULL){
        line = strtok_r(ptr, "\n", &line_buffer);
        if(line == NULL)
            break;

        tokenize_and_parse_line(line);
    }
}

void preprocess_text(char *text, size_t *size)
{
    merge_continued_lines(text, size);
    tokenize_and_parse_text(text, size);
}

void preprocess_file(struct strbuf *filename, struct stat *file_stat)
{
    char *fcontent = __mmap_file(filename->buf, file_stat->st_size);
    size_t size = file_stat->st_size;


    preprocess_text(fcontent, &size);
    printf("%s:\n%s\n", filename->buf, fcontent);
    munmap(fcontent, file_stat->st_size);
}

void preprocess_directory(struct strbuf *path, struct stat *dir_stat)
{
    size_t base_path_len = 0;
    struct dirent *dir_entry;
    struct stat entry_stat;  // contains the current dir_entry stat
    DIR *dirp = opendir(path->buf);

    if(dirp == NULL)
        die(LOG_ERRNO("opendir"));

    // if the dirname doesn't end with a slash, add it
    // /foo/foo -> /foo/foo/
    if(path->buf[path->length-1] != '/')
        strbuf_append_char(path, '/');

    // we save the length of the current path because
    // later we are going to append to this string from this
    // saved index
    base_path_len = path->length;

    while((dir_entry=readdir(dirp))){
        // safe file/folder names that should not be processed
        if(
            !strcmp(dir_entry->d_name, ".") ||
            !strcmp(dir_entry->d_name, "..") ||
            is_in_preignore(dir_entry->d_name)
        )
            continue;

        // we add the current entry name to the dirname at path
        // base_dirname/ + dir_entry->d_name
        strbuf_replace(path, base_path_len, dir_entry->d_name);

        // get the current entry stat
        if(stat(path->buf, &entry_stat) == -1)
            die(LOG_ERROR("stat dir-content", "couldn't get %s entry stat"), path->buf);
        __preprocess_entry(path, &entry_stat);
    }
}

void preprocess_entry(struct strbuf *entry_name, struct stat *estat)
{
    // for now, pass the given entry information to a private function that
    // will handle the entry base on the entry type
    __preprocess_entry(entry_name, estat);
}
