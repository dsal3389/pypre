#include <stdio.h>
#include <string.h>

#include "config.h"
#include "token.h"
#include "utils.h"


static struct token tokens[] = {
    { .name="define", .handler=NULL, .type=TOKEN_TYPE_LINE  },
    { .name="ifndef", .handler=NULL, .type=TOKEN_TYPE_BLOCK }
};


struct token *get_token(struct strbuf_list *tokenized_line)
{
    const struct strbuf *token = NULL;
    int i = 0, j = 0;

    for(; i<tokenized_line->count; i++){
        token = tokenized_line->strings[i];

        if(!strcmp(token->buf, global_config.preprocess_char))
            printf("preprocess char\n");
    }

    return NULL;
}

