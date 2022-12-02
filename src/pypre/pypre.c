#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#include "common.h"
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
        "\t-k,--keep-comments\tkeep comments on processed files\n"
        "\n"
    , progname);
}

/*
parse the program argc, argv, also add all the unknown argv options
to the given entires_buffer, assuming those are filenames
*/
void parse_args(int *argc, char ***argv, linked_list *entries_buffer)
{
    while(**argv){
        if(!strcmp(**argv, "-h") || !strcmp(**argv, "--help"))
            phelp();
        else if(!strcmp(**argv, "-k") || !strcmp(**argv, "--keep-comments"))
            printf("keep comments options enabled\n");
        else{
            // if first charachter in the current argument is -, its
            // probably a flag, because there is no file that start with this char
            if(***argv == '-')
                die(LOG_ERROR("options", "unknown option %s"), **argv);

            // add the current argument to he entries buffer, because it doesn't
            // look like an option, add to the size +1 for the null terminator
            add_entry(entries_buffer, **argv, strlen(**argv) + 1);
        }
        (*argv)++;
        (*argc)--;
    }
}

int run_on_entries(linked_list *entries)
{
    link_entry *entry = entries->head;
    struct stat estat;

    while(entry){
        if(stat((char *) entry->value, &estat) != 0)
            die(LOG_ERROR("stat", "couldn't get information for file named %s"), (char *) entry->value);
        preprocess_entry((char *) entry->value, &estat);
        entry=entry->next;
    }
    return 0;
}

int main(int argc, char **argv)
{
    linked_list entry_names = LINKED_LIST_INIT;
    int results = 0;

    progname = argv[0];
    argv++;
    argc--;

    if(argc == 0)
        pusage();

    parse_args(&argc,  &argv, &entry_names);

    // no files were passed to the program
    if(entry_names.count == 0)
        pusage();

    results = run_on_entries(&entry_names);
    free_list_entries(&entry_names);
    return results;
}
