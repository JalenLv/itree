#include <stdlib.h>
#include <ncurses.h>
#include <unistd.h>

#include "tui.h"
#include "helpers.h"

int run_tui(FileTreeNode *root) {
    // Init curses
    // When input is piped, stdin is not connected to the terminal.
    // We need to open /dev/tty directly for keyboard input.
    FILE *tty_input = fopen("/dev/tty", "r");
    if (tty_input == NULL) {
        // Fallback: if we can't open /dev/tty, skip TUI mode
        fprintf(stderr, "Error: Cannot open /dev/tty for keyboard input.\n");
        return 1;
    }
    
    // Initialize ncurses with the TTY input
    SCREEN *screen = newterm(NULL, stdout, tty_input);
    if (screen == NULL) {
        fprintf(stderr, "Error: Failed to initialize ncurses terminal.\n");
        fclose(tty_input);
        return 1;
    }
    set_term(screen);
    
    // Curses settings
    keypad(stdscr, TRUE);
    cbreak();
    noecho();
    nodelay(stdscr, FALSE); // Blocking input
    curs_set(0); // Hide cursor
    intrflush(stdscr, FALSE); // Don't flush on interrupt keys
    scrollok(stdscr, FALSE); // Disable scrolling

    AppState app_state = {0};
    if (init_app_state(&app_state, root) != 0) {
        fprintf(stderr, "Error: Failed to initialize application state.\n");
        return 1;
    }

    if (draw_visible_entries(&app_state) != 0) {
        fprintf(stderr, "Error: Failed to draw visible entries.\n");
        return 1;
    }

    // Main loop
    int ch;
    while ((ch = getch()) != 'q') { // Press 'q' to quit
        switch (ch) {
            case KEY_DOWN:
            case 'j':
                if (app_state.all_entries.count <= LINES) {
                    // Loop back to top if at the end
                    if (app_state.selected_entry + 1 < app_state.all_entries.count) {
                        app_state.selected_entry++;
                    } else {
                        app_state.selected_entry = 0;
                    }
                } else {
                    if (app_state.selected_entry == app_state.visible_entries_head + app_state.visible_entries_count - 1) {
                        if (app_state.selected_entry == app_state.all_entries.count - 1) {
                            // Loop back to top
                            app_state.selected_entry = 0;
                            app_state.visible_entries_head = 0;
                        } else {
                            // Slide window down
                            app_state.selected_entry++;
                            app_state.visible_entries_head++;
                        }
                    } else {
                        app_state.selected_entry++;
                    }
                }
                break;
            case KEY_UP:
            case 'k':
                if (app_state.all_entries.count <= LINES) {
                    // Loop back to bottom if at the top
                    if (app_state.selected_entry > 0) {
                        app_state.selected_entry--;
                    } else {
                        app_state.selected_entry = app_state.all_entries.count - 1;
                    }
                } else {
                    if (app_state.selected_entry == app_state.visible_entries_head) {
                        if (app_state.selected_entry == 0) {
                            // Loop back to bottom
                            app_state.selected_entry = app_state.all_entries.count - 1;
                            app_state.visible_entries_head = app_state.all_entries.count - app_state.visible_entries_count;
                        } else {
                            // Slide window up
                            app_state.selected_entry--;
                            app_state.visible_entries_head--;
                        }
                    } else {
                        app_state.selected_entry--;
                    }
                }
                break;
            case KEY_LEFT:
            case 'h': // Collapse directory
                FileTreeNode *current_node = DA_GET(FileTreeNode *, &(app_state.all_entries), app_state.selected_entry);
                if (current_node->type == DIRECTORY_NODE && !current_node->collapsed) {
                    current_node->collapsed = 1;
                    // Update all_entries
                    int i = app_state.selected_entry + 1;
                    while (i < app_state.all_entries.count) {
                        FileTreeNode *node = DA_GET(FileTreeNode *, &(app_state.all_entries), i);
                        if (node->depth <= current_node->depth) {
                            break;
                        }
                        i++;
                    }
                    DA_REMOVE_RANGE(FileTreeNode *, &(app_state.all_entries), app_state.selected_entry + 1, i);
                    // Update visible fields
                    app_state.visible_entries_count = MIN(app_state.all_entries.count, LINES);
                    if (app_state.all_entries.count <= LINES) {
                        app_state.visible_entries_head = 0;
                    } else {
                        app_state.visible_entries_head = MIN(app_state.visible_entries_head, app_state.all_entries.count - app_state.visible_entries_count);
                    }
                }
                break;
            case KEY_RIGHT:
            case 'l': // Expand directory
                current_node = DA_GET(FileTreeNode *, &(app_state.all_entries), app_state.selected_entry);
                if (current_node->type == DIRECTORY_NODE && current_node->collapsed) {
                    current_node->collapsed = 0;
                    // Search children rooted at current_node to insert
                    Entries new_entries = {0};
                    init_all_entries_recursive(&new_entries, current_node);
                    DA_REMOVE(FileTreeNode *, &new_entries, 0); // Remove the current_node itself
                    // Insert new entries into all_entries
                    DA_INSERT(FileTreeNode *, &(app_state.all_entries), app_state.selected_entry + 1, &new_entries);
                    // Update visible fields
                    app_state.visible_entries_count = MIN(app_state.all_entries.count, LINES);
                }
                break;
            case 'g': // Go to top
                app_state.selected_entry = 0;
                app_state.visible_entries_head = 0;
                break;
            case 'G': // Go to bottom
                app_state.selected_entry = app_state.all_entries.count - 1;
                app_state.visible_entries_head = MAX(0, app_state.all_entries.count - app_state.visible_entries_count);
                break;
            default:
                break;
        }
        if (draw_visible_entries(&app_state) != 0) {
            fprintf(stderr, "Error: Failed to draw visible entries.\n");
            return 1;
        }
    }

    endwin();
    delscreen(screen);
    fclose(tty_input);
    return 0;
}

int init_all_entries_recursive(Entries *entries, FileTreeNode *node) {
    // Add the current node
    DA_PUSH(FileTreeNode *, entries, node);

    // Add any child if it's a directory and not collapsed
    if (node->type == DIRECTORY_NODE && !node->collapsed) {
        for (int i = 0; i < node->children.count; ++i) {
            FileTreeNode *child = DA_GET(FileTreeNode *, &(node->children), i);
            init_all_entries_recursive(entries, child);
        }
    }

    return 0;
}

int init_app_state(AppState *app_state, FileTreeNode *root) {
    // Init all entries
    init_all_entries_recursive(&(app_state->all_entries), root);
    
    // Init visible related fields
    app_state->visible_entries_head = 0;
    app_state->visible_entries_count = MIN(app_state->all_entries.count, LINES);
    app_state->selected_entry = app_state->visible_entries_head;

    return 0;
}

int draw_visible_entries(AppState *app_state) {
    clear();

    // Draw each visible entry
    for (int row = 0; row < app_state->visible_entries_count; ++row) {
        size_t entry_index = app_state->visible_entries_head + row;
        FileTreeNode *node = DA_GET(FileTreeNode *, &(app_state->all_entries), entry_index);

        // Highlight the selected entry
        if (entry_index == app_state->selected_entry) {
            attron(A_STANDOUT);
            printw("->");
        } else {
            printw("  ");
        }

        // Indentation based on depth
        for (int d = 0; d < node->depth; ++d) {
            printw("    ");
        }

        // Display node name with prefix and suffix
        char *prefix = (node->type == DIRECTORY_NODE) ? (node->collapsed ? "  > " : "  v ") : "    ";
        char *suffix = (node->type == DIRECTORY_NODE) ? "/" : "";
        printw("%s%s%s\n", prefix, node->name, suffix);

        if (entry_index == app_state->selected_entry) {
            attroff(A_STANDOUT);
        }
    }

    refresh();
    return 0;
}
