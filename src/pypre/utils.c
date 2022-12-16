#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "utils.h"
#include "common.h"


static void inline __strbuf_increase(struct strbuf *strbuf, size_t size)
{
    if(size == 0)
        // it is better to add more memory if we know we are going
        // to need more then the default chunk size, so we wont
        // have to allocate memory everytime
        size = STRBUF_INCREASE_CHUNK_SIZE;

    strbuf->capacity += size; 
    strbuf->buf = safe_realloc(strbuf->buf, strbuf->capacity);
}

void strbuf_set(struct strbuf *strbuf, const char *str)
{
    size_t str_len = strlen(str);

    if(str_len >= strbuf->capacity)
        __strbuf_increase(strbuf, str_len*2);

    strncpy(strbuf->buf, str, str_len);
    strbuf->buf[str_len] = 0;
    strbuf->length = str_len;
}

void strbuf_append(struct strbuf *strbuf, const char *str)
{
    size_t str_len = strlen(str);

    if((strbuf->capacity - strbuf->length) < str_len)
        __strbuf_increase(strbuf, str_len);

    strbuf->length += str_len;
    strncat(strbuf->buf, str, strbuf->length);
}

void strbuf_push(struct strbuf *strbuf, const char *str)
{
    size_t str_len = strlen(str);

    if((strbuf->capacity - strbuf->length) < str_len)
        __strbuf_increase(strbuf, str_len);

    // pust the current chars in the buffer X bytes
    memmove(&strbuf->buf[str_len], strbuf->buf, strbuf->length + 1);

    // update the length and copy the given string
    // to the start of the buffer, where we pushed the chars
    strncpy(strbuf->buf, str, str_len);
    strbuf->length += str_len;
}

void strbuf_push_char(struct strbuf *strbuf, int c)
{
    if(strbuf->capacity >= strbuf->length)
        __strbuf_increase(strbuf, 16);

    memmove(strbuf->buf+1, strbuf->buf, strbuf->length + 1);
    strbuf->length++;
    *strbuf->buf = c;
}

void strbuf_append_char(struct strbuf *strbuf, int c)
{
    if(strbuf->capacity >= strbuf->length)
        __strbuf_increase(strbuf, 16);

    strbuf->buf[strbuf->length++] = c;
    strbuf->buf[strbuf->length] = 0;
}

void strbuf_replace(struct strbuf *strbuf, long index, const char *str)
{
    if(strbuf->length < index)
        return;
        
    size_t replace_str_len = 0;
    size_t str_len = strlen(str);
    char *replace_str = &(strbuf->buf[index]);

    if(*replace_str != 0)
        replace_str_len = strlen(replace_str);

    if(
        //str_len > replace_str_len && 
        (strbuf->capacity - strbuf->length) < (str_len - replace_str_len)
    )
        __strbuf_increase(strbuf, (str_len - replace_str_len));

    if(str_len > replace_str_len)
        strbuf->length += (str_len - replace_str_len);
    else
        strbuf->length += (replace_str_len - str_len);
    
    // add extra one, to copy also the null terminator
    strncpy(replace_str, str, str_len + 1);
}

void strbuf_delete(struct strbuf *strbuf, size_t start, size_t count)
{
    if(
        strbuf->length < start || 
        strbuf->length < (start + count)
    )
        return;

    char *start_str = &strbuf->buf[start];
    char *end_str = start_str + count;
    strbuf->length -= count;
    
    memmove(start_str, end_str, strbuf->length + 1);
}

void strbuf_free(struct strbuf *strbuf)
{
    if(strbuf->capacity == 0)
        return;

    strbuf->length = 0;
    strbuf->capacity = 0;
    free(strbuf->buf);
}

void strbuf_list_append(struct strbuf_list *strls, const char *str)
{
    struct strbuf **strbuf;

    // if capacity is equal to count it means the list
    // have no move space, we need to allocate more space
    if(strls->capacity == strls->count){
        strls->capacity += 5;
        strls->strings = safe_realloc(strls->strings, strls->capacity * sizeof(char *));
    }  

    // get the last index (also increase count), allocate memory for
    // the new strbuf and set its inital value
    strbuf = &strls->strings[strls->count++];
    *strbuf = (struct strbuf*) safe_calloc(1, sizeof(struct strbuf));
    strbuf_set(*strbuf, str);
}

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
