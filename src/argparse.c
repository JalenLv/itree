#include <stdio.h>
#include <getopt.h>

#include "argparse.h"

int parse_args(int argc, char *argv[], Args *args) {
    args->path = ".";
    args->output_file = NULL;
    args->show_help = 0;

    int opt;
    static struct option long_options[] = {
        {"output", required_argument, 0, 'o'},
        {"help",   no_argument,       0, 'h'},
        {0, 0, 0, 0}
    };
    
    while ((opt = getopt_long(argc, argv, "o:h", long_options, NULL)) != -1) {
        switch (opt) {
            case 'o':
                args->output_file = optarg;
                break;
            case 'h':
                args->show_help = 1;
                break;
            case '?':
                // getopt_long already printed error message
                return 1;
            default:
                return 1;
        }
    }
    
    // Handle path argument if provided
    int remaining = argc - optind;
    if (remaining > 1) {
        fprintf(stderr, "Error: at most one directory can be given.\n");
        return 1;
    }
    if (remaining == 1) {
        args->path = argv[optind];
    }
    
    return 0;
}

void print_help() {
    printf("Usage: itree [directory] [options]\n");
    printf("       directory defaults to \".\" if not given.\n");
    printf("\nOptions:\n");
    printf("  -o, --output <file>    Specify output file (defaults to stdout)\n");
    printf("  -h, --help             Show this help message\n");
}