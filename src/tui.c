#include <stdlib.h>
#include <unistd.h>

#ifdef WIDE_NCURSES
#include <ncursesw/ncurses.h>
#include <locale.h>
#include <wchar.h>
#else
#include <ncurses.h>
#endif

#include "tui.h"
#include "helpers.h"

void update_tail_given_head(AppState *app_state, FileTree *file_tree) {
    int vis_ent_cnt = 0;
    int i = app_state->visible_entries_head, next_i;
    while ((next_i = next(file_tree, i)) != 0) {
        if (++vis_ent_cnt >= LINES) break;
        i = next_i;
    }
    app_state->visible_entries_tail = i;
}

void update_head_given_tail(AppState *app_state, FileTree *file_tree) {
    int vis_ent_cnt = 0;
    int i = app_state->visible_entries_tail;
    while (i != 0) {
        if (++vis_ent_cnt >= LINES) break;
        i = prev(file_tree, i);
    }
    app_state->visible_entries_head = i;
}

int init_app_state(AppState *app_state, FileTree *file_tree) {
    // Init all entries
    app_state->all_entries = file_tree;

    // Init visible related fields
    app_state->visible_entries_head = 0;
    app_state->selected_entry = 0;
    update_tail_given_head(app_state, file_tree);

    return 0;
}

int draw_visible_entries(AppState *app_state) {
    clear();

    // Draw each visible entry
    int row = 0;
    int i = app_state->visible_entries_head;
    do {
        // Do the drawing
        FileTreeNode *node = DA_GET_PTR(FileTreeNode *, app_state->all_entries, i);
        // Highlight the selected entry
        if (i == app_state->selected_entry) {
            attron(A_STANDOUT);
            mvprintw(row, 0, "->");
        } else {
            mvprintw(row, 0, "  ");
        }
        // Indentation based on depth
        for (int d = 0; d < node->depth; ++d) {
            printw("    ");
        }
        // Display node name with prefix and suffix
        char *prefix = (node->type == DIRECTORY_NODE) ? (node->collapsed ? "  > " : "  v ") : "    ";
        char *suffix = (node->type == DIRECTORY_NODE) ? "/" : (node->type == LINK_NODE ? " -> " : "");
#ifdef WIDE_NCURSES
        // Use wide print for potential wide characters
        if (node->type == LINK_NODE) {
            wchar_t wname[512];
            wchar_t wtarget[512];
            size_t name_ret = mbstowcs(wname, node->name, 512);
            size_t target_ret = mbstowcs(wtarget, node->target, 512);
            if (name_ret == (size_t)(-1) || target_ret == (size_t)(-1)) {
                // Conversion error, fallback to narrow print
                printw("%s%s%s%s\n", prefix, node->name, suffix, node->target);
            } else {
                printw("%s%ls%s%ls\n", prefix, wname, suffix, wtarget);
            }
        } else {
            wchar_t wname[512];
            mbstowcs(wname, node->name, 512);
            printw("%s%ls%s\n", prefix, wname, suffix);
        }
#else
        printw("%s%s%s%s\n", prefix, node->name, suffix, (node->type == LINK_NODE ? node->target : ""));
#endif
        // Turn off highlight if needed
        if (i == app_state->selected_entry) {
            attroff(A_STANDOUT);
        }
        row++;

        // Update i
        i = next(app_state->all_entries, i);
    } while (i != 0 && i <= app_state->visible_entries_tail);

    refresh();
    return 0;
}

int run_tui(FileTree *file_tree) {
    // Init curses
#ifdef WIDE_NCURSES
    setlocale(LC_ALL, "");
#endif
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
    nodelay(stdscr, FALSE);     // Blocking input
    curs_set(0);                // Hide cursor
    intrflush(stdscr, FALSE);   // Don't flush on interrupt keys
    scrollok(stdscr, FALSE);    // Disable scrolling

    AppState app_state = {0};
    if (init_app_state(&app_state, file_tree) != 0) {
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
                if (app_state.visible_entries_head == 0 && next(file_tree, app_state.visible_entries_tail) == 0) {
                    // If the window shows all entries
                    app_state.selected_entry = next(file_tree, app_state.selected_entry);
                } else {
                    // If the window shows a subset of entries
                    if (app_state.selected_entry == app_state.visible_entries_tail) {
                        // Slide window down
                        if (next(file_tree, app_state.visible_entries_tail) != 0) {
                            // If not at the end
                            app_state.visible_entries_head = next(file_tree, app_state.visible_entries_head);
                            app_state.visible_entries_tail = next(file_tree, app_state.visible_entries_tail);
                            app_state.selected_entry = app_state.visible_entries_tail;  
                        } else {
                            // At the end, loop back to top
                            init_app_state(&app_state, file_tree);
                        }
                    } else {
                        // Move selection down
                        app_state.selected_entry = next(file_tree, app_state.selected_entry);
                    }
                }
                break;
            case KEY_UP:
            case 'k':
                if (app_state.visible_entries_head == 0 && next(file_tree, app_state.visible_entries_tail) == 0) {
                    // If the window shows all entries
                    app_state.selected_entry = prev(file_tree, app_state.selected_entry);
                } else {
                    // If the window shows a subset of entries
                    if (app_state.selected_entry == app_state.visible_entries_head) {
                        // Slide window up
                        if (app_state.visible_entries_head != 0) {
                            // If not at the start
                            app_state.visible_entries_tail = prev(file_tree, app_state.visible_entries_tail);
                            app_state.visible_entries_head = prev(file_tree, app_state.visible_entries_head);
                            app_state.selected_entry = app_state.visible_entries_head;  
                        } else {
                            // At the start, go to bottom
                            app_state.visible_entries_tail = prev(file_tree, 0);
                            app_state.selected_entry = app_state.visible_entries_tail;
                            update_head_given_tail(&app_state, file_tree);
                        }
                    } else {
                        // Move selection up
                        app_state.selected_entry = prev(file_tree, app_state.selected_entry);
                    }
                }
                break;
            case KEY_LEFT:
            case 'h': // Collapse directory
                FileTreeNode *current_node = DA_GET_PTR(FileTreeNode *, app_state.all_entries, app_state.selected_entry);
                if (current_node->type == DIRECTORY_NODE && !current_node->collapsed) {
                    current_node->collapsed = 1;
                    // Update visible fields
                    update_tail_given_head(&app_state, file_tree);
                }
                break;
            case KEY_RIGHT:
            case 'l': // Expand directory
                current_node = DA_GET_PTR(FileTreeNode *, app_state.all_entries, app_state.selected_entry);
                if (current_node->type == DIRECTORY_NODE && current_node->collapsed) {
                    current_node->collapsed = 0;
                    // Update visible fields
                    update_tail_given_head(&app_state, file_tree);
                }
                break;
            case KEY_RESIZE:
                update_tail_given_head(&app_state, file_tree);
                break;
            case 'g': // Go to top
                init_app_state(&app_state, file_tree);
                break;
            case 'G': // Go to bottom
                app_state.visible_entries_tail = prev(file_tree, 0);
                app_state.selected_entry = app_state.visible_entries_tail;
                update_head_given_tail(&app_state, file_tree);
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
