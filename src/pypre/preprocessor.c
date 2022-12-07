#include <linux/limits.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>

#include "common.h"
#include "parser.h"
#include "preprocessor.h"
#include "config.h"
#include "utils.h"


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
    if(estat->st_mode & S_IFREG)
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
void merge_continued_lines(const char *fname, char *content, size_t *size)
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
                //*cptr == global_config.line_break_char && 
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

        // update the size of the file, based on the pointers position,
        // we count how many pointers we advanced, and we subtract the difference 
        // because we are about to delete those chars
        *size += tmp - cptr;

        // delete all chars from where we found the line break
        // up to the new line, this is the actual merge
        memmove(tmp, cptr, *size);
        cptr = tmp;
    }
}

/* parsing each line on the given string */
void tokenize_and_parse_text(char *text, size_t *size)
{
    char local_text[*size];
    char *cptr = local_text, *tmp = NULL, token[256];

    // move everything to a local buffer, where we can change the
    // text without effecting the original
    strncpy(local_text, text, *size);

    while(*cptr){
        if(*cptr != global_config.preprocess_char){
            cptr++;
            continue;
        }

        tmp = cptr;
        cptr++;

        while(*cptr == ' ' || *cptr == '\t')
            cptr++;
        
        if(*cptr == '\n' || *cptr == 0)
            break;

        // TODO: think about elegant way to take 
        // the token after the preprocess_char was found
    }
}

void preprocess_file(const char *file_name, struct stat *file_stat)
{
    char *fcontent = __mmap_file(file_name, file_stat->st_size);
    size_t size = file_stat->st_size;

    merge_continued_lines(file_name, fcontent, &size);
    tokenize_and_parse_text(fcontent, &size);

    printf("%s:\n%s\n", file_name, fcontent);
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
            !strcmp(dir_entry->d_name, "..")
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
