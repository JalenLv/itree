#ifndef ARGPARSE_H
#define ARGPARSE_H

/**
 * Structure to hold command line arguments.
 *
 * input_file:  Path to the input file (fall back to stdin).
 * output_file: Path to the output file (fall back to stdout).
 * no_print:    Flag to indicate if printing is disabled.
 * show_help:   Flag to indicate if help message should be shown.
 */
typedef struct {
    char *path;
    char *output_file;
    int no_print;
    int show_help;
} Args;

/**
 * Parses command line arguments and populates the Args structure.
 *
 * Returns 0 on success, non-zero on failure.
 */
int parse_args(int argc, char *argv[], Args *args);

/**
 * Prints the help message to stdout.
 */
void print_help();

#endif // ARGPARSE_H