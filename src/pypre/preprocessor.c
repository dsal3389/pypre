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


#define LINE_BREAK   '\\'
#define COMMENT_CHAR '#'


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

static int __preprocess_entry(const char *ename, struct stat *estat)
{
    if(estat->st_mode & S_IFREG)
        preprocess_file(ename, estat);
    else if(estat->st_mode & S_IFDIR)
        preprocess_directory(ename, estat);
    else
        return -1;
    return 0;
}

/*
merge_continued_lines:
this function convert lines with break char to a single line
for example:
    #define true\
    # True

converted to 
    #define true True
*/
void merge_continued_lines(const char *fname, char *content, size_t size)
{
    char *tmp = NULL, *cptr = content;
    char lcount = 1;  // line count, for readable error messages
    int is_comment = 0;

BEGIN:
    while(*cptr){
        if(*cptr == COMMENT_CHAR){
            is_comment = 1;

            // if the current line is a comment, we check if that comment is continued
            // if it is, we parse this line and set the is_comment to 1, if the comment
            // is not continued (we know it if we see \n), then stop persing this line
            while(*cptr){
                if(*cptr == LINE_BREAK){
                    // if we found a line break, but right after that there is another one,
                    // it means it escape sequance, for example:
                    // #define foo '\\'
                    if(*(cptr+1) == LINE_BREAK){
                        cptr += 2;  // skip 2 because we know the next char is also going to be a line break
                    } else 
                        break;
                }else if(*cptr == '\n'){
                    is_comment = 0;
                    cptr++;
                    lcount++;
                    goto BEGIN;
                }
                cptr++;
            }
        } else if(*cptr != LINE_BREAK){
            if(*cptr == '\n'){
                is_comment = 0;
                lcount++;
            }
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
                    // if the current parsed line is not a comment, then its probably code
                    // that contain \, for example, print("hello\n"), we should stop parse it
                    if(!is_comment)
                        goto BEGIN;
                    die(LOG_ERROR("%s", "unexpected token on line %d (%c)"), fname, lcount, *cptr);
            }
        }

        cptr++;
        lcount++;
        // if after the new line the first char is #
        // then we want to include it also in when we delete
        // chars
        if(*cptr == COMMENT_CHAR)
            cptr++;

        // delete all chars from where we found the line break
        // up to the new line, this is the actual merge
        memmove(tmp, cptr, strlen(cptr) + 1);
        cptr = tmp;
    }
}

void preprocess_file(const char *file_name, struct stat *file_stat)
{
    char *fcontent = __mmap_file(file_name, file_stat->st_size);

    merge_continued_lines(file_name, fcontent, file_stat->st_size);

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

        if(__preprocess_entry(path, &entry_stat) != 0)
            die(LOG_ERROR("process-entry", "failed to process entry, %s, maybe unsupported filetype"), path);
    }
}

void preprocess_entry(const char *ename, struct stat *estat)
{
    // for now, pass the given entry information to a private function that
    // will handle the entry base on the entry type
    if(__preprocess_entry(ename, estat) != 0)
        die(LOG_ERROR("process-entry", "failed to process entry, %s, maybe unsupported filetype"), ename);
}
