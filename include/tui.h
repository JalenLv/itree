#ifndef TUI_H
#define TUI_H

#include "file_tree.h"
#include "helpers.h"

typedef struct {
    DA_FIELDS(FileTreeNode *);
} Entries;

typedef struct AppState {
    Entries all_entries;

    size_t visible_entries_head;
    size_t visible_entries_count;
    size_t selected_entry; // Line number of the selected entry
} AppState;

/**
 * Enters the TUI mode with the given file tree root.
 * Returns 0 on success, non-zero on failure.
 */
int run_tui(FileTreeNode *root);

int init_app_state(AppState *app_state, FileTreeNode *root);

int init_all_entries_recursive(Entries *entries, FileTreeNode *node);

int draw_visible_entries(AppState *app_state);

#endif // TUI_H