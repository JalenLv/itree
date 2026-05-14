#ifndef ARGPARSE_H
#define ARGPARSE_H

#define ITREE_VERSION_MAJOR 0
#define ITREE_VERSION_MINOR 2
#define ITREE_VERSION_PATCH 1

/**
 * Structure to hold command line arguments.
 *
 * path:         Path to the directory to be treed (defaults to ".").
 * output_file:  Path to the output file (fall back to stdout).
 * show_hidden:  If hidden files should be included in the tree.
 * no_tui:       If TUI should be disabled.
 * show_help:    If help message should be shown.
 * show_version: If version message should be shown.
 */
typedef struct {
    char   *path;
    char   *output_file;
    int     show_hidden;
    int     no_tui;
    int     show_help;
    int     show_version;
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

/**
 * Prints the version message to stdout.
 */
void print_version();

#endif // ARGPARSE_H
