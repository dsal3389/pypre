#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>

#include "config.h"
#include "utils.h"


struct config global_config = {
    .preprocess_char='#',
    .line_break_char='\\',
    .output_dirname=OUTPUT_DIRNAME,
    .suppress_warns=0,
    .preignore=STRBUF_LIST_INIT,
};


void __preignore_text(char *text)
{
    char *ptr = NULL, *line = NULL;

    for(ptr=text; ; ptr=NULL){
        line = strtok(ptr, "\n");
        if(line == NULL)
            break;
            
        strbuf_list_append(&global_config.preignore, line);
    }
}

void __preignore()
{
    int fd = open(PREIGNORE_FILENAME, O_RDONLY);
    char *preignore_text;

    if(fd != -1){
        off_t fsize = lseek(fd, 0, SEEK_END);

        if(fsize){
            preignore_text = mmap(NULL, fsize, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
            __preignore_text(preignore_text);
            munmap(preignore_text, fsize);
        }
    } 

    close(fd);
}

int is_in_preignore(const char *name)
{
    struct strbuf * strbuf = NULL;
    
    for(int i=0; i<global_config.preignore.count; i++){
        strbuf = global_config.preignore.strings[i];
        
        if(!strcmp(strbuf->buf, name))
            return 1;
    }
    return 0;
}

void config_init()
{
    __preignore();
}
