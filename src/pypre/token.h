#ifndef _TOKEN_H_ 
#define _TOKEN_H_ 1


#define TOKEN_TYPE_BLOCK 0x1
#define TOKEN_TYPE_LINE 0x2


struct token {
    void (*handler)();
    const char *name;
    int type;
};


extern struct token *get_token(const char *);
extern void handle_token(struct token *, const char *);


#endif
