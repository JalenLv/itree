#ifndef ARGPARSE_H
#define ARGPARSE_H

/**
 * Structure to hold command line arguments.
 *
 * path:        Path to the directory to be treed (defaults to ".").
 * output_file: Path to the output file (fall back to stdout).
 * show_hidden: If hidden files should be included in the tree.
 * no_tui:      If TUI should be disabled.
 * show_help:   If help message should be shown.
 */
typedef struct {
    char   *path;
    char   *output_file;
    int     show_hidden;
    int     no_tui;
    int     show_help;
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