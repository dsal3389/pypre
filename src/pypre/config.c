#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>

#include "config.h"
#include "utils.h"


struct config global_config = {
    .preprocess_char="#",
    .line_break_char='\\',
    .output_dirname=OUTPUT_DIRNAME,
    .suppress_warns=0,
    .preignore=STRBUF_LIST_INIT,
};

void __preignore()
{
    printf("should\n");
    FILE *preignore = fopen(PREIGNORE_FILENAME, "r");

    if(preignore == NULL) return;
    strbuf_list_from_file(&global_config.preignore, preignore);
    fclose(preignore);
}

int should_be_ignored(const char *path)
{
    struct strbuf * strbuf = NULL;
    
    for(int i=0; i<global_config.preignore.count; i++){
        strbuf = global_config.preignore.strings[i];
        
        if(!strcmp(strbuf->buf, path))
            return 1;
    }
    return 0;
}

void config_init()
{
    __preignore();
}
