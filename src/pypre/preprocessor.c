#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>

#include "preprocessor.h"
#include "common.h"
#include "config.h"
#include "token.h"
#include "utils.h"


static void __preprocess_entry(struct strbuf *entry_name, struct stat *estat)
{
    if(estat->st_mode & S_IFREG){
        FILE *file = fopen(entry_name->buf, "r");
        if(file == NULL)
            die(LOG_ERRNO("open"));
        preprocess_file(file, entry_name);
        fclose(file);
    } else if(estat->st_mode & S_IFDIR)
        preprocess_directory(entry_name, estat);
    else
        die(LOG_ERROR("process-entry", "failed to process entry, %s, maybe unsupported filetype"), entry_name->buf);
}

void merge_continued_lines(const char *filename, struct strbuf_list *lines)
{
    struct strbuf *line = NULL, *next_line = NULL;
    char *first_brk = NULL, *last_brk = NULL, merge_index = 0, *c = NULL;
    int should_merge = 1;

    // going in reverse order on the lines, from bottom to top
    for(int i=lines->count-1; i >= 0; i--){
        should_merge = 1;
        line = lines->strings[i];

        if(!line->length) continue;

        if((first_brk=strchr(line->buf, global_config.line_break_char)) == NULL)
            continue;
        
        if((last_brk=strrchr(line->buf, global_config.line_break_char)) == NULL)
            continue;
        
        // if first found brk char is not the same as last brk char it means
        // we have such lines
        // hell\o wo\rld\\,
        // #define 
        //
        // we need to find a way to check if we need to merge the lines or
        // its just an escape sequance
        while(first_brk != last_brk){
            if(*first_brk == global_config.line_break_char)
                should_merge = !should_merge;
            first_brk++;
        }

        if(!should_merge)
            continue;

        // we validate that there arent any special chars after
        // the line break, here is an example for a line that should not be merged
        // print("hello world\n")
        //
        // there is a line break, but its an escape char \n, thats what
        // we are checking here
        c = last_brk+1;
        while(*c){
            switch(*c){
                case ' ':
                case '\t':
                    c++;
                    break;
                default:
                    warn(
                        LOG_WARN(
                            "%s", "unexpected char (%c) after line break, ignoring and not merging line\n"
                            " %d | %s\n"
                        ), filename, *c, i, line->buf
                    );
                    should_merge = 0;
                    goto OUT_C_LOOP;
            }
        }

OUT_C_LOOP:
        if(!should_merge)
            continue;

        // if we need to merge this line, and there is no next line
        // it means we are at the end of the file
        if(i + 1 >= lines->count)
            die(
                LOG_ERROR(
                    "%s", "unexpected line break at the end of the file\n"
                    " > %d | %s\n"
                ), filename, i+1, line->buf
            );

        // remove the break char from the current line
        strbuf_delete(line, last_brk - line->buf, 1);
        next_line = lines->strings[i+1];

        // merge the next line (the line below) and deleted it from
        // the lines list
        if(next_line->length)
            strbuf_append(line, next_line->buf);
        strbuf_list_remove(lines, i+1);
    }
}

void preprocess_text(const char *filename, struct strbuf_list *lines)
{
    merge_continued_lines(filename, lines);
}

void preprocess_file(FILE *file, struct strbuf *filename)
{
    struct strbuf_list lines = STRBUF_LIST_INIT;

    strbuf_list_from_file(&lines, file);
    preprocess_text(filename->buf, &lines);
    for(int i=0; i<lines.count; i++)
        printf("%d | %s\n", i, lines.strings[i]->buf);
    strbuf_list_free(&lines);
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
            should_be_ignored(dir_entry->d_name)
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
