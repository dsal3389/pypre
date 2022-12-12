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
        __strbuf_increase(strbuf, 0);

    strncpy(strbuf->buf, str, str_len);
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

void strbuf_append_char(struct strbuf *strbuf, int c)
{
    if((strbuf->capacity - strbuf->length) < 1)
        __strbuf_increase(strbuf, 0);

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
        str_len > replace_str_len && 
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
