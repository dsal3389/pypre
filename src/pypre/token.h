#ifndef _TOKEN_H_ 
#define _TOKEN_H_ 1


struct token_handler {
    const char *token;
    void (*handler)();
};


void handle_token(const char *);


#endif
