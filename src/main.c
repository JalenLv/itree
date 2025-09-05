#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "argparse.h"
#include "cJSON.h"
#include "helpers.h"
#include "file_tree.h"

// Helpers for file I/O
int open_io(Args args, FILE **input, FILE **output);
void close_io(FILE *input, FILE *output);

int main(int argc, char *argv[]) {
    // Parse args
    Args args;
    if (parse_args(argc, argv, &args) != 0) {
        print_help();
        return 1;
    }

    // Handle help flag
    if (args.show_help) {
        print_help();
        return 0;
    }

    // Open input/output files
    FILE *input, *output;
    if (open_io(args, &input, &output) != 0) {
        return 1;
    }

    // Create file tree
    cJSON *file_tree = create_file_tree(input);
    if (file_tree == NULL) {
        fprintf(stderr, "Error: Failed to parse JSON input.\n");
        close_io(input, output);
        return 1;
    }
    printf("%s\n", cJSON_Print(file_tree));

    // Clean up
    close_io(input, output);
    return 0;
}

int open_io(Args args, FILE **input, FILE **output) {
    // Handle input/output files
    *input = stdin;
    *output = stdout;
    if (args.input_file) {
        *input = fopen(args.input_file, "r");
        if (!*input) {
            fprintf(stderr, "Error: Cannot open input file: %s\n", args.input_file);
            return 1;
        }
    }
    if (args.output_file) {
        *output = fopen(args.output_file, "w");
        if (!*output) {
            fprintf(stderr, "Error: Cannot open output file: %s\n", args.output_file);
            if (*input != stdin) fclose(*input);
            return 1;
        }
    }
    return 0;
}

void close_io(FILE *input, FILE *output) {
    if (input != stdin) fclose(input);
    if (output != stdout) fclose(output);
}
