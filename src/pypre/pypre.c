#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "common.h"
#include "config.h"
#include "preprocessor.h"
#include "utils.h"


void pusage()
{
    die("%s [options...] [files...]\n", progname);
}

void phelp()
{
    die(
        "%s [options...] [files...]\n"
        "\n"
        "options:\n"
        "\t-h,--help\tprint this message to screen\n"
        "\t-s,--suppress-warns\tdon't print warning messages\n"
        "\t-d,--output-directory\twhere to write the preprocessed files"
        "\n"
    , progname);
}

/*
parse the program argc, argv, also add all the unknown argv options
to the given entires_buffer, assuming those are filenames
*/
void parse_args(int *argc, char ***argv, struct strbuf_list *entries_buffer)
{
    while(**argv){
        if(!strcmp(**argv, "-h") || !strcmp(**argv, "--help"))
            phelp();
        else if(!strcmp(**argv, "-s") || !strcmp(**argv, "--suppress-warns"))
            global_config.suppress_warns = 1;
        else if(!strcmp(**argv, "-d") || !strcmp(**argv, "--output-directory")){
            if(*argc < 2)
                die(LOG_ERROR("options", "missing value for %s"), **argv);
            (*argv)++;
            (*argc)--;
            if(strlen(**argv) == 0)
                die(LOG_ERROR("options", "output directory cannot be an empty string"));
            global_config.output_dirname = **argv;
        } else{
            // if first charachter in the current argument is -, its
            // probably a flag, because there is no file that start with this char
            if(***argv == '-')
                die(LOG_ERROR("options", "unknown option %s"), **argv);

            // add the current argument to he entries buffer, because it doesn't
            // look like an option, add to the size +1 for the null terminator
            strbuf_list_append(entries_buffer, **argv);
        }
        (*argv)++;
        (*argc)--;
    }
}

/*
read data from stdin and parse the text, all the parsed
text will be written to stdout
*/
int run_on_stdin()
{
    struct strbuf input = STRBUF_INIT;
    char c;

    while((c = getchar()) != EOF)
        strbuf_append_char(&input, c);

    preprocess_text("stdin", input.buf, &input.length);
    printf("%s", input.buf);
    strbuf_free(&input);
    return 0;
}

/*
run the preprocessor on the given entries, those entries
can be files or directories
*/
int run_on_entries(struct strbuf_list *entries)
{
    struct strbuf *strbuf = NULL;
    struct stat estat;

    for(int i=0; i<entries->count; i++){
        strbuf = entries->strings[i];

        if(stat(strbuf->buf, &estat) != 0)
            die(LOG_ERROR("stat", "couldn't get information for file named %s"), strbuf->buf);
        preprocess_entry(strbuf, &estat);
    }
    return 0;
}

int main(int argc, char **argv)
{
    struct strbuf_list entry_names = STRBUF_LIST_INIT;
    int results = 0;

    progname = argv[0];
    argv++;
    argc--;

    if(argc != 0)
        parse_args(&argc,  &argv, &entry_names);
    config_init();

    // no files were passed to the program
    if(entry_names.count == 0)
        results = run_on_stdin();
    else
        results = run_on_entries(&entry_names);

    strbuf_list_free(&entry_names);
    return results;
}
