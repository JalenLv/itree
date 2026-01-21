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

    // Create file tree
    FileTree file_tree = {0};
    if (create_file_tree_from_path(&file_tree, args.path)) {
        fprintf(stderr, "Error: Failed to create file tree from path: %s\n", args.path);
        return 1;
    }

    // Run TUI
    if (run_tui(&file_tree) != 0) {
        fprintf(stderr, "Error: Failed in TUI.\n");
        DA_FREE(FileTree, &file_tree);
        return 1;
    }

    if (args.no_print == 0) {
        // Open output file
        FILE *output = NULL;
        if (open_io(&args, &output) != 0) {
            fprintf(stderr, "Error: Failed to open output file.\n");
            DA_FREE(FileTree, &file_tree);
            return 1;
        }

        // Draw tree
        if (draw_tree(&file_tree, output) != 0) {
            fprintf(stderr, "Error: Failed to draw file tree.\n");
            close_io(output);
            DA_FREE(FileTree, &file_tree);
            return 1;
        }

        close_io(output);
    }

    // Clean up
    DA_FREE(FileTree, &file_tree);
    return 0;
}
