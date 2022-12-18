#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <strings.h>
#include <sys/stat.h>

#include "utils.h"
#include "common.h"


static void __strbuf_increase(struct strbuf *strbuf, size_t size)
{
    if(size == 0)
        // it is better to add more memory if we know we are going
        // to need more then the default chunk size, so we wont
        // have to allocate memory everytime
        size = STRBUF_INCREASE_CHUNK_SIZE;
    
    strbuf->capacity += size; 
    strbuf->buf = (char *) safe_realloc(strbuf->buf, strbuf->capacity*sizeof(char));

    // initilize the new allocated memory with 0
    memset(&strbuf->buf[strbuf->capacity - size], 0, size*sizeof(char));
}

static void __strbuf_list_increase(struct strbuf_list *strls, size_t size)
{
    strls->capacity += size;
    strls->strings = safe_realloc(strls->strings, strls->capacity*sizeof(void*));

    for(int i=size; i; i--)
        strls->strings[strls->capacity - i] = (struct strbuf*) safe_calloc(1, sizeof(struct strbuf));
}

void strbuf_from_file(struct strbuf *strbuf, FILE *file)
{
    long fsize = 0, tmp = 0;
    char buffer[256];

    tmp = ftell(file);
    fseek(file, 0, SEEK_SET);
    fseek(file, 0, SEEK_END);
    fsize = ftell(file);

    // go back to the original offset
    fseek(file, tmp, SEEK_SET);

    if(fsize >= strbuf->capacity)
        __strbuf_increase(strbuf, fsize+1);
    
    while(fgets(buffer,  sizeof(buffer), file) != NULL)
        strbuf_append(strbuf, buffer);

    fread(strbuf->buf, sizeof(char), fsize, file);
    strbuf->length = fsize;
}

/* set the strbuf value with the given string */
void strbuf_set(struct strbuf *strbuf, const char *str)
{
    size_t str_len = strlen(str);

    if(str_len >= strbuf->capacity)
        __strbuf_increase(strbuf, str_len*2);

    strncpy(strbuf->buf, str, str_len);
    strbuf->length = str_len;
    strbuf->buf[strbuf->length] = 0;
}

/* append a string to the given strbuf */
void strbuf_append(struct strbuf *strbuf, const char *str)
{
    size_t str_len = strlen(str);

    if((strbuf->capacity - strbuf->length) <= str_len)
        __strbuf_increase(strbuf, str_len+1);

    strncat(strbuf->buf, str, str_len);
    strbuf->length += str_len;
    strbuf->buf[strbuf->length] = 0;
}

/* push string to the start of the strbuf */
void strbuf_push(struct strbuf *strbuf, const char *str)
{
    size_t str_len = strlen(str);

    if((strbuf->capacity - strbuf->length) < str_len)
        __strbuf_increase(strbuf, str_len + 1);

    // pust the current chars in the buffer X bytes
    memmove(&strbuf->buf[str_len], strbuf->buf, strbuf->length + 1);

    // update the length and copy the given string
    // to the start of the buffer, where we pushed the chars
    strncpy(strbuf->buf, str, str_len+1);
    strbuf->length += str_len;
}

/* push char to the start of the strbuf */
void strbuf_push_char(struct strbuf *strbuf, int c)
{
    if(strbuf->capacity <= strbuf->length)
        __strbuf_increase(strbuf, 16);

    memmove(strbuf->buf+1, strbuf->buf, strbuf->length + 1);
    strbuf->length++;
    *strbuf->buf = c;
}

/* appends a char to the end of the strbuf */
void strbuf_append_char(struct strbuf *strbuf, int c)
{
    if(strbuf->capacity <= strbuf->length)
        __strbuf_increase(strbuf, 16);

    strbuf->buf[strbuf->length++] = c;
    strbuf->buf[strbuf->length] = 0;
}

/* replace a string on the given index with the given str */
void strbuf_replace(struct strbuf *strbuf, long index, const char *str)
{
    if(strbuf->length < index)
        return;
        
    size_t replace_str_len = 0;
    size_t str_len = strlen(str);
    char *replace_str = &(strbuf->buf[index]);

    if(*replace_str != 0)
        replace_str_len = strlen(replace_str);

    if((strbuf->capacity - strbuf->length) < (str_len - replace_str_len))
        __strbuf_increase(strbuf, (str_len - replace_str_len));

    if(str_len > replace_str_len)
        strbuf->length += (str_len - replace_str_len);
    else
        strbuf->length += (replace_str_len - str_len);
    
    // add extra one, to copy also the null terminator
    strncpy(replace_str, str, str_len);
}

/* delete X chars from the given index on the strbuf */
void strbuf_delete(struct strbuf *strbuf, size_t start, size_t count)
{
    if(strbuf->length <= start)
        return;

    if(strbuf->length < (start + count))
        count = strbuf->length - start - 1;

    char *start_str = &strbuf->buf[start];
    char *end_str = start_str + count;
    
    memmove(start_str, end_str, strbuf->length - start);
    strbuf->length -= count;
}

/* free all the allocated memory for the strbuf */
void strbuf_free(struct strbuf *strbuf)
{
    if(strbuf->capacity == 0)
        return;

    strbuf->length = 0;
    strbuf->capacity = 0;
    free(strbuf->buf);
}

/* read from the given stream, each line in the stream is an strbuf */
void strbuf_list_from_file(struct strbuf_list *strls, FILE *file)
{
    char buffer[256];
    int line_length = 0, is_new_line = 0;

    while(fgets(buffer, sizeof(buffer)-1, file) != NULL){
        is_new_line = 0;
        line_length = strlen(buffer);

        if(strls->count >= strls->capacity)
            __strbuf_list_increase(strls, 32);

        if(buffer[line_length-1] == '\n'){
            is_new_line = 1;
            // if the current line length is 1 its probably an empty line,
            // a line with only \n, we want to include it in our string_list but we
            // cannot use strbuf_append on it (the lines added to strbuf_list should not
            // contain \n char)
            if(line_length == 1)
                goto LOOP_END;
            buffer[line_length-1] = 0;
        }

        strbuf_append(strls->strings[strls->count], buffer);

LOOP_END:
        if(is_new_line)
            strls->count++;
    }

    // we should add to the count 1, because we start from 0,
    // so if for example a file with 1 line, the strls->count will be 0, 
    // because there was only 1 line
    if(strls->count)
        strls->count++;
}

/* appends strbuf to the strbuf_list */
void strbuf_list_append(struct strbuf_list *strls, const char *str)
{
    struct strbuf **strbuf;

    // if capacity is equal to count it means the list
    // have no move space, we need to allocate more space
    if(strls->capacity <= strls->count)
        __strbuf_list_increase(strls, 5);

    // get the last index (also increase count), allocate memory for
    // the new strbuf and set its inital value
    strbuf = &strls->strings[strls->count++];
    strbuf_set(*strbuf, str);
}

/* remove strbuf on the given index from the strbuf_list */
void strbuf_list_remove(struct strbuf_list *strls, size_t index)
{
    if(index >= strls->count)
        return;
    
    struct strbuf **strbuf = &strls->strings[index];
    strbuf_free(*strbuf);
    strls->count--;

    memmove(strbuf, strbuf+1, (strls->count - index) * sizeof(void*));
}

/* free all the allocated memory for the strbuf_list and the strbuf strings */
void strbuf_list_free(struct strbuf_list *strls)
{
    for(int i=0; i<strls->count; i++)
        strbuf_free(strls->strings[i]);
        
    strls->count = 0;
    strls->capacity = 0;
    free(strls->strings);
}

/*
reads the first work in the given text and stores it into
the buffer, the function returns a pointer to the end of the found word
in the given text, usage example:

    char line[] = "Hello world",
    char *ptr = line;
    struct strbuf word;

    while(ptr != NULL){
        ptr = get_next_word(&word, ptr);
        printf("word %s\n", word->buf);
    }
*/
char *get_next_word(struct strbuf *strbuf, char *text)
{
    char *start = NULL, *end = text;
    char tmp_c;

    while(*end == ' '){
        // if we didn't find any letter char until now, and we got to
        // the end of the string, then it means the given text does not
        // contain any word
        if(*end == 0)
            return NULL;
        end++;
    }

    // record the start of the word
    start = end;

    // read until we meat a seperator, then it
    // means we got to the end of the word
    while(*end){
        switch(*end){
            case '\n':
            case '\t':
            case ' ':
                goto LOOP_OUT;
            default:
                break;
        }
        end++;
    }

LOOP_OUT:
    // remember the current end char for later,
    // we set the null terminator so `strbuf_set` will only add the found
    // word, after that we place the `tmp_c` back
    tmp_c = *end;
    *end = 0;
    strbuf_set(strbuf, start);

    // if the end of the word is NULL terminator, then
    // it means we reached the end of the text
    if(tmp_c == 0)
        return NULL;

    *end = tmp_c;
    return end;
}

void create_directories(struct strbuf *path)
{
    struct strbuf current_dir = STRBUF_INIT;
    char path_buffer[path->length];
    char *path_ptr = path_buffer, *dirname = NULL;

    strncpy(path_buffer, path->buf, path->length + 1);
    
    if(!strncmp(path_ptr, "./", 2)){
        strbuf_append(&current_dir, "./");
        path_ptr += 2;
    }

    for(;; path_ptr=NULL){
        dirname = strtok(path_ptr, "/");
        if(dirname == NULL)
            break;

        strbuf_append(&current_dir, dirname);
        strbuf_append_char(&current_dir, '/');
        printf("%s\n", current_dir.buf);

        mkdir(current_dir.buf, 0777);
    }

    strbuf_free(&current_dir);
}
