#ifndef _CONFIG_H_ 
#define _CONFIG_H_ 1 


struct config{
    char preprocess_char;
    char line_break_char;
    int suppress_warns;
};


extern struct config global_config;


#endif
