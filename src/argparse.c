#include <stdio.h>
#include <getopt.h>

#include "argparse.h"

int parse_args(int argc, char *argv[], Args *args) {
    args->path         = ".";
    args->output_file  = NULL;
    args->no_tui       = 0;
    args->show_hidden  = 0;
    args->show_help    = 0;
    args->show_version = 0;

    enum { OPT_NO_TUI = 1000 };

    int opt;
    static struct option long_options[] = {
        {"output",      required_argument, 0, 'o'},
        {"help",        no_argument,       0, 'h'},
        {"version",     no_argument,       0, 'v'},
        {"show-hidden", no_argument,       0, 'a'},
        {"no-tui",      no_argument,       0, OPT_NO_TUI},
        {0, 0, 0, 0}
    };
    
    while ((opt = getopt_long(argc, argv, "o:hva", long_options, NULL)) != -1) {
        switch (opt) {
            case 'o': {
                args->output_file = optarg;
                break;
            }
            case 'a': {
                args->show_hidden = 1;
                break;
            }
            case OPT_NO_TUI: {
                args->no_tui = 1;
                break;
            }
            case 'h': {
                args->show_help = 1;
                break;
            }
            case 'v': {
                args->show_version = 1;
                break;
            }
            case '?':
            case ':':
            default: {
                return 1;
            }
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

void print_version() {
    printf("itree v%d.%d.%d © 2025 - 2026 by Jalen Lyu\n",
           ITREE_VERSION_MAJOR, ITREE_VERSION_MINOR, ITREE_VERSION_PATCH);
}

void print_help() {
    printf("Usage: itree [directory] [options]\n");
    printf("       directory defaults to \".\" if not given.\n");
    printf("\nOptions:\n");
    printf("  -o, --output <file>    Specify output file (defaults to stdout)\n");
    printf("  -a, --show-hidden      Include hidden files in the tree\n");
    printf("      --no-tui           Disable TUI mode\n");
    printf("  -h, --help             Show this help message\n");
    printf("  -v, --version          Show version information\n");
}