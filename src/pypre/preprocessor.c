#include <linux/limits.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>

#include "common.h"
#include "preprocessor.h"
#include "config.h"


static char *__mmap_file(const char *path, size_t size)
{
    char *content = NULL;
    int fd = open(path, O_RDWR);

    if(fd == -1)
        die(LOG_ERRNO("open"));
    content = (char *) safe_mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
    close(fd);
    return content;
}

static void __preprocess_entry(const char *ename, struct stat *estat)
{
    if(estat->st_mode & (S_IFREG | S_IFBLK | S_IFCHR))
        preprocess_file(ename, estat);
    else if(estat->st_mode & S_IFDIR)
        preprocess_directory(ename, estat);
    else
        die(LOG_ERROR("process-entry", "failed to process entry, %s, maybe unsupported filetype"), ename);
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
void merge_continued_lines(const char *fname, char *content, size_t size)
{
    char *tmp = NULL, *cptr = content;
    char lcount = 1;  // line count, for readable error messages

BEGIN:
    while(*cptr){
        if(
            *cptr != global_config.line_break_char || 
            // if the current char is "\"" and the next char is also "\"
            // it meants its an escape seqance "\\"
            (
                *cptr == global_config.line_break_char && 
                *(cptr + 1) == global_config.line_break_char
            )
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
                        LOG_WARN("unexpected char (%c) after line break, on line %d, ignoring"), *cptr, lcount
                    );
                    goto BEGIN;
            }
        }

        cptr++;
        lcount++;
        // if after the new line the first char is #
        // then we want to include it also in when we delete
        // chars
        if(*cptr == global_config.preprocess_char)
            cptr++;

        // delete all chars from where we found the line break
        // up to the new line, this is the actual merge
        memmove(tmp, cptr, strlen(cptr) + 1);
        cptr = tmp;
    }
}

/* 
parsing a preprocessor line, a line that starts with global_config.preprocess_char,
returns a pointer to the end on the line, including the \n
*/
char *parse_preprocessor_line(char *token)
{
    char *cptr = token;

    while(*cptr && *cptr != '\n'){
        printf("%c", *cptr);
        cptr++;
    }
    
    putchar('\n');
    if(*cptr == '\n')
        cptr++;
    return cptr;
}

/* parsing each line on the given string */
void parse_string(char *str)
{
    char *cptr = str;
    char *tmp = NULL;

    while(*cptr){
        if(*cptr != global_config.preprocess_char){
            cptr++;
            continue;
        }
        
        tmp = cptr;
        cptr = parse_preprocessor_line(cptr);

        // delete the whole preprocessor line
        memmove(tmp, cptr, strlen(cptr) + 1);
        cptr = tmp;
    }
}

void preprocess_file(const char *file_name, struct stat *file_stat)
{
    char *fcontent = __mmap_file(file_name, file_stat->st_size);

    merge_continued_lines(file_name, fcontent, file_stat->st_size);
    parse_string(fcontent);

    //printf("%s:\n%s\n", file_name, fcontent);
    munmap(fcontent, file_stat->st_size);
}

void preprocess_directory(const char *dirname, struct stat *dir_stat)
{
    char path[PATH_MAX], *path_post_suffix = NULL;
    struct dirent *dir_entry;
    struct stat entry_stat;  // contains the current dir_entry stat
    DIR *dirp = opendir(dirname);

    if(dirp == NULL)
        die(LOG_ERRNO("opendir"));

    // the first entry in the path must be the current
    // directory name
    strncpy(path, dirname, sizeof(path));

    // get the address after the current directory name
    // in the path variable, path: "foo/ "
    //                                  ^ we are here
    path_post_suffix = &path[strlen(path)];

    // if the dirname doesn't end with a slash, add it
    // /foo/foo -> /foo/foo/
    if(*(path_post_suffix-1) != '/'){
        *path_post_suffix = '/';
        path_post_suffix++;
    }

    while((dir_entry=readdir(dirp))){
        // safe file/folder names that should not be processed
        if(
            !strcmp(dir_entry->d_name, ".") ||
            !strcmp(dir_entry->d_name, "..") ||
            !strcmp(dir_entry->d_name, "__pycache__")
        )
            continue;

        // we add the current entry name to the dirname at path
        // dirname/ + dir_entry->d_name
        strncpy(path_post_suffix, dir_entry->d_name, sizeof(path) - strlen(dirname));

        // get the current entry stat
        if(stat(path, &entry_stat) == -1)
            die(LOG_ERROR("stat dir-content", "couldn't get %s entry stat"), path);
        __preprocess_entry(path, &entry_stat);
    }
}

void preprocess_entry(const char *ename, struct stat *estat)
{
    // for now, pass the given entry information to a private function that
    // will handle the entry base on the entry type
    __preprocess_entry(ename, estat);
}
