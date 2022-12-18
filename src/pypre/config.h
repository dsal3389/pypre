#ifndef _CONFIG_H_ 
#define _CONFIG_H_ 1 

#include "utils.h"


#define PREIGNORE_FILENAME ".pypreignore"
#define OUTPUT_DIRNAME "dist"


struct config{
    char preprocess_char;
    char line_break_char;
    char *output_dirname;
    int suppress_warns;
    struct strbuf_list preignore;
};


extern struct config global_config;
extern int should_be_ignored(const char *);
extern void config_init();


#endif
