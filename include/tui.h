#ifndef TUI_H
#define TUI_H

#include "file_tree.h"

typedef struct {
    FileTree *all_entries;          // All file tree entries

    size_t visible_entries_head;    // Index of the first visible entry
    size_t visible_entries_tail;    // Index of the last visible entry
    size_t selected_entry;          // Index of the currently selected entry
} AppState;

/**
 * Enters the TUI mode with the given file tree root.
 * Returns 0 on success, non-zero on failure.
 */
int run_tui(FileTree *file_tree);

#endif // TUI_H