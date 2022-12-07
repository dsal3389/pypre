#include <stdio.h>
#include <string.h>

#include "config.h"
#include "parser.h"
#include "utils.h"


struct token_handler{
    char *name;
    void (*handler)(char *);
};


void handler_if(char *text);


static struct token_handler handlers[] = {
    { .name="if", .handler=handler_if },
    { .name="ifndef", .handler=handler_if },
};


int parse_token(char *token, char *text)
{
    struct token_handler *handler = handlers;

    for(int i=0; i<sizeof(handlers) / sizeof(struct token_handler); i++){
        if(strcmp(token, handler->name)){
            handler->handler(text);
            return 0;
        }
        handler++;
    }
    return -1;
}

void handler_if(char *text)
{
    printf("hello world\n");
}
