#include <stdio.h>
#include <string.h>

#include "token.h"


#define REGISTER_HANDLER(token_) { .token=#token_, .handler=token_ ## _handler }


void define_handler()
{
    printf("Hello world\n");
}


static struct token_handler handlers[] = {
    REGISTER_HANDLER(define),
};


#define HANDLERS_COUNT sizeof(handlers) / sizeof(struct token_handler)


void handle_token(const char *token)
{
    struct token_handler *handler = NULL; 

    for(int i=0; i<HANDLERS_COUNT; i++){
        handler = &handlers[i];

        if(!strcmp(token, handler->token)){
            handler->handler();
            return;
        }
    }
}
