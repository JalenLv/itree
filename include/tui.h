#ifndef TUI_H
#define TUI_H

#include "file_tree.h"

#ifdef WIDE_NCURSES
#include <ncursesw/ncurses.h>
#include <locale.h>
#include <wchar.h>
#else
#include <ncurses.h>
#endif

typedef struct {
    FileTree *all_entries;      // All file tree entries

    int rows;                   // Ncurses LINES value
    int cols;                   // Ncurses COLS value
    int visible_entries_head;   // Index of the first visible entry
    int visible_entries_tail;   // Index of the last visible entry
    int selected_entry;         // Index of the currently selected entry

    int col_offset;             // Horizontal scroll offset for the tree view
    WINDOW *tree_pad;           // Pad for drawing the file tree
    int tree_pad_rows;          // # of rows in the tree pad
    int tree_pad_cols;          // # of columns in the tree pad
} AppState;

/**
 * Enters the TUI mode with the given file tree root.
 * Returns 0 on success, non-zero on failure.
 */
int run_tui(FileTree *file_tree);

/**
 * Renders the whole TUI.
 * Responsible for erasing and refrashing the screen.
 * Dispatches to draw_visible_entries().
 * Returns 0 on success, non-zero on failure.
 */
int render_tui(AppState *app_state);

/**
 * Draws the visible entries in the tree pad.
 * Returns 0 to keep running, non-zero on failure.
 */
int draw_visible_entries(AppState *app_state);

/**
 * Draws the overflow indicator at each line where the entry name exceeds the current column width.
 * Returns 0 on success, non-zero on failure.
 */
int draw_overflow_indicator(AppState *app_state);

/**
 * Draws the arrow pointing to the selected entry.
 * Returns 0 on success, non-zero on failure.
 */
int draw_selected_entry_arrow(AppState *app_state);

/**
 * Do what their names suggest.
 */
void update_tail_given_head(AppState *app_state);
void update_head_given_tail(AppState *app_state);
int  init_app_state(AppState *app_state, FileTree *file_tree, int rows);

/**
 * Computes the necessary size of the tree pad based on the visible entries.
 * Updates the tree pad if necessary by reallocating it with newpad().
 * New tree pad is pointed to by app_state->tree_pad, caller's responsible to check if it's NULL.
 */
void update_tree_pad(AppState *app_state);

/**
 * Handles a single keystroke against the application state.
 * Returns 0 to keep running, 1 to quit.
 */
int handle_key(AppState *app_state, int ch);

#endif // TUI_H
