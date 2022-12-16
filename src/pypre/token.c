#include <stdio.h>
#include <string.h>

#include "token.h"


static struct token tokens[] = {
    { .name="define", .handler=NULL, .type=TOKEN_TYPE_LINE  },
    { .name="ifndef", .handler=NULL, .type=TOKEN_TYPE_BLOCK }
};


#define TOKENS_LEN sizeof(tokens) / sizeof(struct token)


struct token *get_token(const char *token)
{
    struct token *current_token = NULL;

    for(int i=0; i<TOKENS_LEN; i++){
        current_token = &tokens[i];

        if(!strcmp(token, current_token->name))
            return current_token;
    }
    return NULL;
}

void handle_token(struct token *token, const char *line)
{

}
