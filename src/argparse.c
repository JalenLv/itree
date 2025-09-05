#include <stdio.h>
#include <getopt.h>

#include "argparse.h"

int parse_args(int argc, char *argv[], Args *args) {
    args->input_file = NULL;
    args->output_file = NULL;
    args->show_help = 0;

    int opt;
    static struct option long_options[] = {
        {"input",  required_argument, 0, 'i'},
        {"output", required_argument, 0, 'o'},
        {"help",   no_argument,       0, 'h'},
        {0, 0, 0, 0}
    };
    
    while ((opt = getopt_long(argc, argv, "i:o:h", long_options, NULL)) != -1) {
        switch (opt) {
            case 'i':
                args->input_file = optarg;
                break;
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
    
    // Check for non-option arguments
    if (optind < argc) {
        fprintf(stderr, "Error: Unexpected argument: %s\n", argv[optind]);
        return 1;
    }
    
    return 0;
}

void print_help() {
    printf("Usage: itree [options] OR USE PIPE tree -J | itree [options]\n");
    printf("Options:\n");
    printf("  -i, --input <file>    Input file\n");
    printf("  -o, --output <file>   Output file\n");
    printf("  -h, --help            Show this help message\n");
}