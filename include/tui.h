#ifndef TUI_H
#define TUI_H

#include "file_tree.h"

typedef struct {
    FileTree *all_entries;      // All file tree entries
    int lines;                  // Ncurses LINES value
    int visible_entries_head;   // Index of the first visible entry
    int visible_entries_tail;   // Index of the last visible entry
    int selected_entry;         // Index of the currently selected entry
} AppState;

/**
 * Enters the TUI mode with the given file tree root.
 * Returns 0 on success, non-zero on failure.
 */
int run_tui(FileTree *file_tree);

/**
 * Pure-logic helpers exposed for unit testing. These do not call ncurses;
 * the `lines` parameter is the visible row count (LINES at runtime).
 */
void update_tail_given_head(AppState *app_state);
void update_head_given_tail(AppState *app_state);
int  init_app_state(AppState *app_state, FileTree *file_tree);

/**
 * Handles a single keystroke against the application state.
 * Returns 0 to keep running, 1 to quit.
 */
int handle_key(AppState *app_state, int ch);

#endif // TUI_H
