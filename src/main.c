#include <stdio.h>

#include "argparse.h"
#include "file_tree.h"
#include "draw_tree.h"
#include "tui.h"
#include "helpers.h"

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

    // Handle version flag
    if (args.show_version) {
        print_version();
        return 0;
    }

    FILE *output = NULL;
    FileTree file_tree = {0};
    // Create file tree
    if (create_file_tree_from_path(&file_tree, &args)) {
        fprintf(stderr, "Error: Failed to create file tree from path: %s\n", args.path);
        goto clear;
    }

    // Run TUI
    if (!args.no_tui && run_tui(&file_tree) != 0) {
        fprintf(stderr, "Error: Failed in TUI.\n");
        goto clear;
    }

    // Open output file
    if (open_io(&args, &output) != 0) {
        fprintf(stderr, "Error: Failed to open output file.\n");
        goto clear;
    }

    // Draw tree
    if (draw_tree(&file_tree, output) != 0) {
        fprintf(stderr, "Error: Failed to draw file tree.\n");
        goto clear;
    }

    // Clean up
    close_io(output);
    FileTree_free(&file_tree);
    return 0;

    // Clean up if error occurs
clear:
    if (output) close_io(output);
    FileTree_free(&file_tree);
    return 1;
}
