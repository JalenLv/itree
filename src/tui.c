#include "tui.h"
#include "helpers.h"
#include <stdlib.h>
#include <unistd.h>

// Helpers' declaration
static int ctrl_d_shift_down_page(AppState *app_state);
static int ctrl_u_shift_up_page(AppState *app_state);
static int handle_mouse(AppState *app_state);
static int mouse_left_click(AppState *app_state, MEVENT *event);
static int mouse_left_double_click(AppState *app_state, MEVENT *event);
static int mouse_scroll_up(AppState *app_state);
static int mouse_scroll_down(AppState *app_state);

static int pad_cell_is_blank(WINDOW *pad, int y, int x);
static void pad_cell_copy(WINDOW *dst, int dy, int dx, WINDOW *src, int sy, int sx);
static int entry_display_width(FileTreeNode *node);

int run_tui(FileTree *file_tree) {
#ifdef WIDE_NCURSES
    setlocale(LC_ALL, "");
#endif
    WINDOW *screen = initscr();
    if (screen == NULL) {
        fprintf(stderr, "Error: Failed to initialize terminal UI.\n");
        return 1;
    }
    keypad(stdscr, TRUE);       // Enable function keys and arrow keys
    cbreak();                   // Disable line buffering
    noecho();                   // Don't echo input
    nodelay(stdscr, FALSE);     // Blocking input
    curs_set(0);                // Hide cursor
    intrflush(stdscr, FALSE);   // Don't flush on interrupt keys
    scrollok(stdscr, FALSE);    // Disable scrolling
    mousemask(ALL_MOUSE_EVENTS | REPORT_MOUSE_POSITION, NULL); // Enable mouse events

    refresh();

    AppState app_state = {0};
    if (init_app_state(&app_state, file_tree, LINES) != 0) {
        fprintf(stderr, "Error: Failed to initialize application state.\n");
        return 1;
    }

    if (render_tui(&app_state) != 0) {
        fprintf(stderr, "Error: Failed to draw visible entries.\n");
        return 1;
    }

    // Main loop
    int ch;
    while (1) {
        ch = getch();
        // TODO("Handle handle_key error");
        if (handle_key(&app_state, ch) != 0) break;
        if (render_tui(&app_state) != 0) {
            fprintf(stderr, "Error: Failed to draw visible entries.\n");
            return 1;
        }
    }

    endwin();
    return 0;
}

int render_tui(AppState *app_state) {
    werase(stdscr);
    wnoutrefresh(stdscr);

    if (draw_visible_entries(app_state) != 0) {
        fprintf(stderr, "Error: Failed to draw visible entries.\n");
        return 1;
    }
    if (draw_overflow_indicator(app_state) != 0) {
        fprintf(stderr, "Error: Failed to draw overflow indicators.\n");
        return 1;
    }
    if (draw_selected_entry_arrow(app_state) != 0) {
        fprintf(stderr, "Error: Failed to draw selected entry arrow.\n");
        return 1;
    }

    doupdate();
    return 0;
}

int draw_visible_entries(AppState *app_state) {
    werase(app_state->tree_pad);

    int row = 0;
    for (int i = app_state->visible_entries_head; i < app_state->visible_entries_tail; i = next(app_state->all_entries, i)) {
        // Do the drawing
        FileTreeNode *node;
        if (FileTree_get_ptr(app_state->all_entries, i, &node) != 0) {
            return 1;
        }
        // Leave 2 spaces on the left for the selected entry arrow
        mvwprintw(app_state->tree_pad, row, 0, "  ");
        // Indentation based on depth
        for (int d = 0; d < node->depth; ++d) {
            wprintw(app_state->tree_pad, "    ");
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
                wprintw(app_state->tree_pad, "%s", prefix);
                if (i == app_state->selected_entry) {
                    wattron(app_state->tree_pad, A_STANDOUT);
                }
                wprintw(app_state->tree_pad, "%s%s%s\n", node->name, suffix, node->target);
            } else {
                wprintw(app_state->tree_pad, "%s", prefix);
                if (i == app_state->selected_entry) {
                    wattron(app_state->tree_pad, A_STANDOUT);
                }
                wprintw(app_state->tree_pad, "%ls%s%ls\n", wname, suffix, wtarget);
            }
        } else {
            wchar_t wname[512];
            size_t name_ret = mbstowcs(wname, node->name, 512);
            if (name_ret == (size_t)(-1)) {
                wprintw(app_state->tree_pad, "%s", prefix);
                if (i == app_state->selected_entry) {
                    wattron(app_state->tree_pad, A_STANDOUT);
                }
                wprintw(app_state->tree_pad, "%s%s\n", node->name, suffix);
            } else {
                wprintw(app_state->tree_pad, "%s", prefix);
                if (i == app_state->selected_entry) {
                    wattron(app_state->tree_pad, A_STANDOUT);
                }
                wprintw(app_state->tree_pad, "%ls%s\n", wname, suffix);
            }
        }
#else
        wprintw(app_state->tree_pad, "%s", prefix);
        if (i == app_state->selected_entry) {
            wattron(app_state->tree_pad, A_STANDOUT);
        }
        wprintw(app_state->tree_pad, "%s%s%s\n", node->name, suffix, (node->type == LINK_NODE ? node->target : ""));
#endif
        // Turn off highlight if needed
        if (i == app_state->selected_entry) {
            wattroff(app_state->tree_pad, A_STANDOUT);
        }

        // Update row
        row++;
    }

    pnoutrefresh(app_state->tree_pad, 0, app_state->col_offset, 0, 0, app_state->rows - 1, app_state->cols - 1);
    return 0;
}

int draw_overflow_indicator(AppState *app_state) {
    if (app_state->tree_pad_cols <= app_state->cols) return 0;

    WINDOW *of_ind_win = newwin(app_state->rows, 1, 0, app_state->cols - 1);
    if (of_ind_win == NULL) {
        fprintf(stderr, "Error: Failed to create overflow indicator window.\n");
        return 1;
    }
    werase(of_ind_win);

    int row = 0;
    for (int i = app_state->visible_entries_head; i < app_state->visible_entries_tail; i = next(app_state->all_entries, i)) {
        int is_overflowing = 0;
        for (int col = app_state->cols + app_state->col_offset; col < app_state->tree_pad_cols; ++col) {
            if (!pad_cell_is_blank(app_state->tree_pad, row, col)) {
                is_overflowing = 1;
                break;
            }
        }
        if (is_overflowing) {
            wattron(of_ind_win, A_STANDOUT);
            mvwaddch(of_ind_win, row, 0, '>');
            wattroff(of_ind_win, A_STANDOUT);
        } else {
            pad_cell_copy(of_ind_win, row, 0,
                          app_state->tree_pad, row,
                          app_state->cols + app_state->col_offset - 1);
        }
        row++;
    }

    wnoutrefresh(of_ind_win);
    delwin(of_ind_win);
    return 0;
}

int draw_selected_entry_arrow(AppState *app_state) {
    int row = 0;
    for (int i = app_state->visible_entries_head; i < app_state->visible_entries_tail; i = next(app_state->all_entries, i)) {
        if (i == app_state->selected_entry) {
            WINDOW *sel_arrow_win = newwin(1, 4, row, 0);
            if (sel_arrow_win == NULL) {
                fprintf(stderr, "Error: Failed to create selected entry arrow window.\n");
                return 1;
            }
            werase(sel_arrow_win);
            wattrset(sel_arrow_win, A_NORMAL);
            mvwprintw(sel_arrow_win, 0, 0, "->  ");
            wnoutrefresh(sel_arrow_win);
            delwin(sel_arrow_win);
            return 0;
        }
        row++;
    }

    return 1; // Unreachable
}

void update_tail_given_head(AppState *app_state) {
    int vis_ent_cnt = 0;
    int i = app_state->visible_entries_head, next_i;
    while ((next_i = next(app_state->all_entries, i)) != FILE_TREE_SENTINEL_TAIL) {
        if (++vis_ent_cnt >= app_state->rows) break;
        i = next_i;
    }
    app_state->visible_entries_tail = next_i;
}

void update_head_given_tail(AppState *app_state) {
    int vis_ent_cnt = 0;
    int i = prev(app_state->all_entries, app_state->visible_entries_tail), prev_i;
    while ((prev_i = prev(app_state->all_entries, i)) != FILE_TREE_SENTINEL_HEAD) {
        if (++vis_ent_cnt >= app_state->rows) break;
        i = prev_i;
    }
    app_state->visible_entries_head = i;
}

void update_tree_pad(AppState *app_state) {
    int max_len = 0;
    for (int i = app_state->visible_entries_head; i < app_state->visible_entries_tail; i = next(app_state->all_entries, i)) {
        FileTreeNode *node;
        if (FileTree_get_ptr(app_state->all_entries, i, &node) != 0) {
            // TODO: make update_tree_pad return int so this error can propagate instead of being swallowed by continue
            continue;
        }
        int len = entry_display_width(node);
        if (len > max_len) max_len = len;
    }

    // Update tree pad if necessary
    if (app_state->tree_pad == NULL ||
        app_state->tree_pad_cols < MAX(max_len, app_state->cols) ||
        app_state->tree_pad_rows < app_state->rows)
    {
        app_state->tree_pad_rows = app_state->rows;
        app_state->tree_pad_cols = MAX(app_state->cols, max_len);
        if (app_state->tree_pad != NULL)
            delwin(app_state->tree_pad);
        app_state->tree_pad = newpad(app_state->tree_pad_rows, app_state->tree_pad_cols);
    }
}

int init_app_state(AppState *app_state, FileTree *file_tree, int rows) {
    app_state->all_entries = file_tree;
    app_state->rows = rows;
    // TODO: Parameterize cols as well for testing purposes; also write tests for small terminal sizes
    app_state->cols = COLS;
    app_state->visible_entries_head = 0;
    app_state->selected_entry = 0;
    update_tail_given_head(app_state);
    app_state->col_offset = 0;
    update_tree_pad(app_state);
    if (app_state->tree_pad == NULL) {
        fprintf(stderr, "Error: Failed to initialize tree pad.\n");
        return 1;
    }
    return 0;
}

int handle_key(AppState *app_state, int ch) {
    switch (ch) {
        case 'q': {
            return 1;
        }
        case KEY_DOWN:
        case 'j': {
            int is_selected_at_bottom = (next(app_state->all_entries, app_state->selected_entry) == app_state->visible_entries_tail);
            int is_window_at_bottom = (app_state->visible_entries_tail == FILE_TREE_SENTINEL_TAIL);
            if (!is_selected_at_bottom) {
                // Move selection down
                app_state->selected_entry = next(app_state->all_entries, app_state->selected_entry);
            } else if (!is_window_at_bottom) {
                // If not at the end, slide window down
                app_state->visible_entries_head = next(app_state->all_entries, app_state->visible_entries_head);
                app_state->visible_entries_tail = next(app_state->all_entries, app_state->visible_entries_tail);
                app_state->selected_entry = next(app_state->all_entries, app_state->selected_entry);
                update_tree_pad(app_state);
                if (app_state->tree_pad == NULL) {
                    fprintf(stderr, "Error: Failed to update tree pad.\n");
                    return 1;
                }
            } else {
                // At the end, loop back to top
                init_app_state(app_state, app_state->all_entries, app_state->rows);
            }
            break;
        }
        case KEY_UP:
        case 'k': {
            int is_selected_at_top = (app_state->selected_entry == app_state->visible_entries_head);
            int is_window_at_top = (app_state->visible_entries_head == 0);
            if (!is_selected_at_top) {
                // Move selection up
                app_state->selected_entry = prev(app_state->all_entries, app_state->selected_entry);
            } else if (!is_window_at_top) {
                // If not at the start, slide window up
                app_state->visible_entries_head = prev(app_state->all_entries, app_state->visible_entries_head);
                app_state->visible_entries_tail = prev(app_state->all_entries, app_state->visible_entries_tail);
                app_state->selected_entry = app_state->visible_entries_head;
                update_tree_pad(app_state);
                if (app_state->tree_pad == NULL) {
                    fprintf(stderr, "Error: Failed to update tree pad.\n");
                    return 1;
                }
            } else {
                // At the start, loop back to bottom
                app_state->visible_entries_tail = FILE_TREE_SENTINEL_TAIL;
                update_head_given_tail(app_state);
                app_state->selected_entry = prev(app_state->all_entries, app_state->visible_entries_tail);
                update_tree_pad(app_state);
                if (app_state->tree_pad == NULL) {
                    fprintf(stderr, "Error: Failed to update tree pad.\n");
                    return 1;
                }
            }
            break;
        }
        case 'h': { // Collapse directory
            FileTreeNode *current_node;
            if (FileTree_get_ptr(app_state->all_entries, app_state->selected_entry, &current_node) != 0) {
                return 1;
            }
            if (current_node->type == DIRECTORY_NODE && !current_node->collapsed) {
                current_node->collapsed = 1;
                update_tail_given_head(app_state);
                update_tree_pad(app_state);
                if (app_state->tree_pad == NULL) {
                    fprintf(stderr, "Error: Failed to update tree pad.\n");
                    return 1;
                }
            }
            break;
        }
        case 'l': { // Expand directory
            FileTreeNode *current_node;
            if (FileTree_get_ptr(app_state->all_entries, app_state->selected_entry, &current_node) != 0) {
                return 1;
            }
            if (current_node->type == DIRECTORY_NODE && current_node->collapsed) {
                current_node->collapsed = 0;
                update_tail_given_head(app_state);
                update_tree_pad(app_state);
                if (app_state->tree_pad == NULL) {
                    fprintf(stderr, "Error: Failed to update tree pad.\n");
                    return 1;
                }
            }
            break;
        }
        case KEY_RESIZE: {
            app_state->rows = LINES;
            app_state->cols = COLS;
            update_tail_given_head(app_state);
            if (app_state->selected_entry >= app_state->visible_entries_tail) {
                app_state->selected_entry = prev(app_state->all_entries, app_state->visible_entries_tail);
            }
            update_tree_pad(app_state);
            if (app_state->tree_pad == NULL) {
                fprintf(stderr, "Error: Failed to update tree pad.\n");
                return 1;
            }
            break;
        }
        case 'g': { // Go to top
            init_app_state(app_state, app_state->all_entries, app_state->rows);
            break;
        }
        case 'G': { // Go to bottom
            app_state->visible_entries_tail = FILE_TREE_SENTINEL_TAIL;
            app_state->selected_entry = prev(app_state->all_entries, app_state->visible_entries_tail);
            update_head_given_tail(app_state);
            update_tree_pad(app_state);
            if (app_state->tree_pad == NULL) {
                fprintf(stderr, "Error: Failed to update tree pad.\n");
                return 1;
            }
            break;
        }
        case 4: // Ctrl-D
        case 'd': { // Go down half a page
            ctrl_d_shift_down_page(app_state);
            break;
        }
        case 21: // Ctrl-U
        case 'u': { // Go up half a page
            ctrl_u_shift_up_page(app_state);
            break;
        }
        case KEY_LEFT: { // Go left half a page
            app_state->col_offset = MAX(0, app_state->col_offset - app_state->cols / 2);
            break;
        }
        case KEY_RIGHT: { // Go right half a page
            app_state->col_offset += app_state->cols / 2;
            break;
        }
        case KEY_MOUSE: {
            handle_mouse(app_state);
            break;
        }
    }
    return 0;
}

// Helpers' implementation
static int ctrl_d_shift_down_page(AppState *app_state) {
    for (int i = 0; i < app_state->rows / 2; ++i) {
        if (app_state->visible_entries_tail != FILE_TREE_SENTINEL_TAIL) {
            // If not at the end, slide window down
            // Keep selected entry relative stationary to the window
            app_state->visible_entries_head = next(app_state->all_entries, app_state->visible_entries_head);
            app_state->visible_entries_tail = next(app_state->all_entries, app_state->visible_entries_tail);
            app_state->selected_entry = next(app_state->all_entries, app_state->selected_entry);
            update_tree_pad(app_state);
            if (app_state->tree_pad == NULL) {
                fprintf(stderr, "Error: Failed to update tree pad.\n");
                return 1;
            }
        } else if (next(app_state->all_entries, app_state->selected_entry) != FILE_TREE_SENTINEL_TAIL) {
            // At the end, slide selected entry down only
            app_state->selected_entry = next(app_state->all_entries, app_state->selected_entry);
        }
    }
    return 0;
}

static int ctrl_u_shift_up_page(AppState *app_state) {
    for (int i = 0; i < app_state->rows / 2; ++i) {
        if (app_state->visible_entries_head != 0) {
            // If not at the start, slide window up
            // Keep selected entry relative stationary to the window
            app_state->visible_entries_head = prev(app_state->all_entries, app_state->visible_entries_head);
            app_state->visible_entries_tail = prev(app_state->all_entries, app_state->visible_entries_tail);
            app_state->selected_entry = prev(app_state->all_entries, app_state->selected_entry);
            update_tree_pad(app_state);
            if (app_state->tree_pad == NULL) {
                fprintf(stderr, "Error: Failed to update tree pad.\n");
                return 1;
            }
        } else if (app_state->selected_entry != 0) {
            // At the start, slide selected entry up only
            app_state->selected_entry = prev(app_state->all_entries, app_state->selected_entry);
        }
    }
    return 0;
}

static int handle_mouse(AppState *app_state) {
    USED(app_state);

    MEVENT event;
    if (getmouse(&event) == OK) {
        if (event.bstate & BUTTON1_CLICKED) {
            // Left click to select entry or click on arrow to select and toggle expansion
            mouse_left_click(app_state, &event);
        } else if (event.bstate & BUTTON1_DOUBLE_CLICKED) { // Left double click to select and toggle expansion
            mouse_left_double_click(app_state, &event);
        } else if (event.bstate & BUTTON4_PRESSED) { // Scroll up 1/10 page
            mouse_scroll_up(app_state);
        } else if (event.bstate & BUTTON5_PRESSED) { // Scroll down 1/10 page
            mouse_scroll_down(app_state);
        } else return 0;
    }
    return 1;
}

static int mouse_left_click(AppState *app_state, MEVENT *event) {
    int click_row = event->y;
    int click_col = event->x;
    if (click_col < 0 || click_col >= app_state->cols || click_row < 0 || click_row >= app_state->rows) return 0;

    int row = 0, i;
    for (i = app_state->visible_entries_head; i < app_state->visible_entries_tail; i = next(app_state->all_entries, i)) {
        if (row == click_row) break;
        row++;
    }
    if (i == app_state->visible_entries_tail) return 0;

    FileTreeNode *clicked_node;
    if (FileTree_get_ptr(app_state->all_entries, i, &clicked_node) != 0) {
        return 1;
    }
    int pos_arrow = 2 + clicked_node->depth * 4 + 2 - app_state->col_offset;
    int pos_begin_name = 2 + clicked_node->depth * 4 + 4 - app_state->col_offset;
    int pos_end_name = entry_display_width(clicked_node) - app_state->col_offset;

    if (click_col >= pos_arrow - 1 && click_col <= pos_arrow + 1 && clicked_node->type == DIRECTORY_NODE) {
        // Click on arrow area, toggle expansion if directory
        clicked_node->collapsed = !clicked_node->collapsed;
        app_state->selected_entry = i;
        update_tail_given_head(app_state);
        update_tree_pad(app_state);
        if (app_state->tree_pad == NULL) {
            fprintf(stderr, "Error: Failed to update tree pad.\n");
            return 1;
        }
    } else if (click_col >= pos_begin_name && click_col < pos_end_name) {
        // Click on name area, select entry
        app_state->selected_entry = i;
    }

    return 0;
}

static int mouse_left_double_click(AppState *app_state, MEVENT *event) {
    int click_row = event->y;
    int click_col = event->x;
    if (click_col < 0 || click_col >= app_state->cols || click_row < 0 || click_row >= app_state->rows) return 0;

    int row = 0, i;
    for (i = app_state->visible_entries_head; i < app_state->visible_entries_tail; i = next(app_state->all_entries, i)) {
        if (row == click_row) break;
        row++;
    }
    if (i == app_state->visible_entries_tail) return 0;

    FileTreeNode *clicked_node;
    if (FileTree_get_ptr(app_state->all_entries, i, &clicked_node) != 0) {
        return 1;
    }
    int pos_begin_name = 2 + clicked_node->depth * 4 + 4 - app_state->col_offset;
    int pos_end_name = entry_display_width(clicked_node) - app_state->col_offset;
    if (click_col >= pos_begin_name && click_col < pos_end_name) {
        // Double click on name area, toggle expansion if directory and select entry
        if (clicked_node->type == DIRECTORY_NODE) {
            clicked_node->collapsed = !clicked_node->collapsed;
            update_tail_given_head(app_state);
            update_tree_pad(app_state);
            if (app_state->tree_pad == NULL) {
                fprintf(stderr, "Error: Failed to update tree pad.\n");
                return 1;
            }
        }
        app_state->selected_entry = i;
    }

    return 0;
}

static int mouse_scroll_up(AppState *app_state) {
    for (int i = 0; i < MAX(app_state->rows / 10, 1); ++i) {
        if (app_state->visible_entries_head != 0) {
            // If not at the start, slide window up
            // Keep selected entry unchanged if it's within the window
            // Otherwise move it with the window
            // Do nothing if at the top
            app_state->visible_entries_head = prev(app_state->all_entries, app_state->visible_entries_head);
            app_state->visible_entries_tail = prev(app_state->all_entries, app_state->visible_entries_tail);
            if (app_state->selected_entry >= app_state->visible_entries_tail) {
                app_state->selected_entry = prev(app_state->all_entries, app_state->visible_entries_tail);
            }
            update_tree_pad(app_state);
            if (app_state->tree_pad == NULL) {
                fprintf(stderr, "Error: Failed to update tree pad.\n");
                return 1;
            }
        }
    }
    return 0;
}

static int mouse_scroll_down(AppState *app_state) {
    for (int i = 0; i < MAX(app_state->rows / 10, 1); ++i) {
        if (app_state->visible_entries_tail != FILE_TREE_SENTINEL_TAIL) {
            // If not at the end, slide window down
            // Keep selected entry unchanged if it's within the window
            // Otherwise move it with the window
            // Do nothing if at the bottom
            app_state->visible_entries_head = next(app_state->all_entries, app_state->visible_entries_head);
            app_state->visible_entries_tail = next(app_state->all_entries, app_state->visible_entries_tail);
            if (app_state->selected_entry < app_state->visible_entries_head) {
                app_state->selected_entry = app_state->visible_entries_head;
            }
            update_tree_pad(app_state);
            if (app_state->tree_pad == NULL) {
                fprintf(stderr, "Error: Failed to update tree pad.\n");
                return 1;
            }
        }
    }
    return 0;
}

static int pad_cell_is_blank(WINDOW *pad, int y, int x) {
#ifdef WIDE_NCURSES
    cchar_t c;
    if (mvwin_wch(pad, y, x, &c) == ERR) return 1;
    wchar_t buf[CCHARW_MAX];
    attr_t attrs;
    short pair;
    if (getcchar(&c, buf, &attrs, &pair, NULL) == ERR) return 1;
    return buf[0] == L' ' || buf[0] == L'\0';
#else
    chtype c = mvwinch(pad, y, x);
    return ((c & A_CHARTEXT) == ' ' || (c & A_CHARTEXT) == ERR);
#endif
}

static void pad_cell_copy(WINDOW *dst, int dy, int dx, WINDOW *src, int sy, int sx) {
#ifdef WIDE_NCURSES
    // If sx lands on the second cell of a 2-column glyph, ncurses fills it
    // with a padding wchar (L'\0'); back off one cell so we don't copy a half-glyph.
    cchar_t c;
    if (mvwin_wch(src, sy, sx, &c) == ERR) {
        mvwaddch(dst, dy, dx, ' ');
        return;
    }
    wchar_t buf[CCHARW_MAX];
    attr_t attrs;
    short pair;
    if (getcchar(&c, buf, &attrs, &pair, NULL) != ERR && buf[0] == L'\0' && sx > 0) {
        if (mvwin_wch(src, sy, sx - 1, &c) == ERR) {
            mvwaddch(dst, dy, dx, ' ');
            return;
        }
    }
    mvwadd_wch(dst, dy, dx, &c);
#else
    chtype c = wmove(src, sy, sx) == ERR ? ' ' : winch(src);
    mvwaddch(dst, dy, dx, c);
#endif
}

static int entry_display_width(FileTreeNode *node) {
    int len = 2 + node->depth * 4 + 4;
    len += (node->type == DIRECTORY_NODE) ? 1 : ((node->type == LINK_NODE) ? 4 : 0);
#ifdef WIDE_NCURSES
    wchar_t wname[512];
    size_t wn = mbstowcs(wname, node->name, 512);
    if (wn == (size_t)(-1)) {
        len += (int)strlen(node->name);
    } else {
        int w = wcswidth(wname, wn);
        len += (w < 0) ? (int)wn : w;
    }
    if (node->type == LINK_NODE) {
        wchar_t wtarget[512];
        size_t wt = mbstowcs(wtarget, node->target, 512);
        if (wt == (size_t)(-1)) {
            len += (int)strlen(node->target);
        } else {
            int w = wcswidth(wtarget, wt);
            len += (w < 0) ? (int)wt : w;
        }
    }
#else
    len += (int)strlen(node->name);
    len += (node->type == LINK_NODE) ? (int)strlen(node->target) : 0;
#endif
    return len;
}
