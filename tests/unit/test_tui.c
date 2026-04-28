#include "greatest.h"
#include "tui.h"
#include "file_tree.h"
#include "test_util.h"

#ifdef WIDE_NCURSES
#include <ncursesw/ncurses.h>
#else
#include <ncurses.h>
#endif

/*
 * Build a flat tree with `n` files under one root (n+1 nodes total).
 * All entries are visible (no collapsed dirs).
 */
static void build_flat_tree(FileTree *tree, int n) {
    synth_push(tree, DIRECTORY_NODE, "root", 0, 0, NULL);
    char name[8];
    for (int i = 0; i < n; ++i) {
        snprintf(name, sizeof(name), "f%02d", i);
        synth_push(tree, FILE_NODE, name, 1, 0, NULL);
    }
}

/* ---------------------------------------------------------------------------
 * init_app_state and the update_* helpers
 * ------------------------------------------------------------------------ */

TEST init_app_state_with_window_smaller_than_tree(void) {
    FileTree tree = {0};
    build_flat_tree(&tree, 7); /* 8 nodes total */
    AppState s = {0};
    ASSERT_EQ(0, init_app_state(&s, &tree, 4 /* lines */));
    ASSERT_EQ(0, s.visible_entries_head);
    ASSERT_EQ(0, s.selected_entry);
    /* tail is the 4th visible entry: index 3 */
    ASSERT_EQ(3, s.visible_entries_tail);
    DA_FREE(FileTreeNode, &tree);
    PASS();
}

TEST init_app_state_with_window_larger_than_tree(void) {
    FileTree tree = {0};
    build_flat_tree(&tree, 3); /* 4 nodes total */
    AppState s = {0};
    ASSERT_EQ(0, init_app_state(&s, &tree, 100));
    ASSERT_EQ(0, s.visible_entries_head);
    ASSERT_EQ(0, s.selected_entry);
    /* tail should be the last node, idx 3 */
    ASSERT_EQ(3, s.visible_entries_tail);
    DA_FREE(FileTreeNode, &tree);
    PASS();
}

/* ---------------------------------------------------------------------------
 * handle_key: navigation
 * ------------------------------------------------------------------------ */

TEST q_returns_one(void) {
    FileTree tree = {0};
    build_flat_tree(&tree, 3);
    AppState s = {0};
    init_app_state(&s, &tree, 10);
    ASSERT_EQ(1, handle_key(&s, &tree, 'q', 10));
    DA_FREE(FileTreeNode, &tree);
    PASS();
}

TEST j_moves_selection_down(void) {
    FileTree tree = {0};
    build_flat_tree(&tree, 5); /* 6 nodes */
    AppState s = {0};
    init_app_state(&s, &tree, 10);
    ASSERT_EQ(0, handle_key(&s, &tree, 'j', 10));
    ASSERT_EQ(1, s.selected_entry);
    ASSERT_EQ(0, handle_key(&s, &tree, 'j', 10));
    ASSERT_EQ(2, s.selected_entry);
    DA_FREE(FileTreeNode, &tree);
    PASS();
}

TEST key_down_arrow_same_as_j(void) {
    FileTree tree = {0};
    build_flat_tree(&tree, 5);
    AppState s = {0};
    init_app_state(&s, &tree, 10);
    ASSERT_EQ(0, handle_key(&s, &tree, KEY_DOWN, 10));
    ASSERT_EQ(1, s.selected_entry);
    DA_FREE(FileTreeNode, &tree);
    PASS();
}

TEST j_at_window_bottom_slides_window(void) {
    FileTree tree = {0};
    build_flat_tree(&tree, 7); /* 8 nodes */
    AppState s = {0};
    init_app_state(&s, &tree, 4); /* window of 4 */
    /* Move to bottom of visible window */
    handle_key(&s, &tree, 'j', 4); /* sel=1 */
    handle_key(&s, &tree, 'j', 4); /* sel=2 */
    handle_key(&s, &tree, 'j', 4); /* sel=3 (bottom) */
    ASSERT_EQ(3, s.selected_entry);
    ASSERT_EQ(0, s.visible_entries_head);
    ASSERT_EQ(3, s.visible_entries_tail);
    /* Next j should slide window down by one */
    handle_key(&s, &tree, 'j', 4);
    ASSERT_EQ(1, s.visible_entries_head);
    ASSERT_EQ(4, s.visible_entries_tail);
    ASSERT_EQ(4, s.selected_entry);
    DA_FREE(FileTreeNode, &tree);
    PASS();
}

TEST j_at_tree_end_wraps_to_top(void) {
    FileTree tree = {0};
    build_flat_tree(&tree, 3); /* 4 nodes, window large enough to fit all */
    AppState s = {0};
    init_app_state(&s, &tree, 100);
    /* Move selection to last node */
    while (s.selected_entry != s.visible_entries_tail) handle_key(&s, &tree, 'j', 100);
    /* When window holds entire tree, j on last selected just wraps via next() */
    handle_key(&s, &tree, 'j', 100);
    ASSERT_EQ(0, s.selected_entry);
    DA_FREE(FileTreeNode, &tree);
    PASS();
}

TEST k_moves_selection_up(void) {
    FileTree tree = {0};
    build_flat_tree(&tree, 5);
    AppState s = {0};
    init_app_state(&s, &tree, 10);
    handle_key(&s, &tree, 'j', 10); /* sel=1 */
    handle_key(&s, &tree, 'j', 10); /* sel=2 */
    ASSERT_EQ(2, s.selected_entry);
    ASSERT_EQ(0, handle_key(&s, &tree, 'k', 10));
    ASSERT_EQ(1, s.selected_entry);
    DA_FREE(FileTreeNode, &tree);
    PASS();
}

TEST key_up_arrow_same_as_k(void) {
    FileTree tree = {0};
    build_flat_tree(&tree, 5);
    AppState s = {0};
    init_app_state(&s, &tree, 10);
    handle_key(&s, &tree, 'j', 10);
    ASSERT_EQ(1, s.selected_entry);
    handle_key(&s, &tree, KEY_UP, 10);
    ASSERT_EQ(0, s.selected_entry);
    DA_FREE(FileTreeNode, &tree);
    PASS();
}

/* ---------------------------------------------------------------------------
 * handle_key: collapse / expand
 * ------------------------------------------------------------------------ */

TEST h_on_directory_collapses(void) {
    FileTree tree = {0};
    synth_push(&tree, DIRECTORY_NODE, "root", 0, 0, NULL);
    synth_push(&tree, DIRECTORY_NODE, "sub",  1, 0, NULL);
    synth_push(&tree, FILE_NODE,      "leaf", 2, 0, NULL);
    AppState s = {0};
    init_app_state(&s, &tree, 10);
    /* Move to "sub" */
    handle_key(&s, &tree, 'j', 10);
    ASSERT_EQ(1, s.selected_entry);
    /* Collapse */
    handle_key(&s, &tree, 'h', 10);
    ASSERT_EQ(1, tree.items[1].collapsed);
    DA_FREE(FileTreeNode, &tree);
    PASS();
}

TEST l_on_collapsed_directory_expands(void) {
    FileTree tree = {0};
    synth_push(&tree, DIRECTORY_NODE, "root", 0, 0, NULL);
    synth_push(&tree, DIRECTORY_NODE, "sub",  1, 1 /* start collapsed */, NULL);
    synth_push(&tree, FILE_NODE,      "leaf", 2, 0, NULL);
    AppState s = {0};
    init_app_state(&s, &tree, 10);
    handle_key(&s, &tree, 'j', 10); /* sel=sub */
    ASSERT_EQ(1, s.selected_entry);
    handle_key(&s, &tree, 'l', 10);
    ASSERT_EQ(0, tree.items[1].collapsed);
    DA_FREE(FileTreeNode, &tree);
    PASS();
}

TEST h_on_file_is_noop(void) {
    FileTree tree = {0};
    build_flat_tree(&tree, 3);
    AppState s = {0};
    init_app_state(&s, &tree, 10);
    handle_key(&s, &tree, 'j', 10); /* sel=1 (a file) */
    int prev_sel = s.selected_entry;
    handle_key(&s, &tree, 'h', 10);
    ASSERT_EQ(prev_sel, s.selected_entry);
    ASSERT_EQ(0, tree.items[1].collapsed);
    DA_FREE(FileTreeNode, &tree);
    PASS();
}

/* ---------------------------------------------------------------------------
 * handle_key: g / G / Ctrl-D / Ctrl-U / KEY_RESIZE
 * ------------------------------------------------------------------------ */

TEST g_resets_to_top(void) {
    FileTree tree = {0};
    build_flat_tree(&tree, 7);
    AppState s = {0};
    init_app_state(&s, &tree, 4);
    /* scroll deep into the tree */
    for (int i = 0; i < 6; ++i) handle_key(&s, &tree, 'j', 4);
    ASSERT(s.selected_entry > 0);
    handle_key(&s, &tree, 'g', 4);
    ASSERT_EQ(0, s.visible_entries_head);
    ASSERT_EQ(0, s.selected_entry);
    DA_FREE(FileTreeNode, &tree);
    PASS();
}

TEST G_jumps_to_bottom(void) {
    FileTree tree = {0};
    build_flat_tree(&tree, 7); /* 8 nodes; last index 7 */
    AppState s = {0};
    init_app_state(&s, &tree, 4);
    handle_key(&s, &tree, 'G', 4);
    ASSERT_EQ(7, s.selected_entry);
    ASSERT_EQ(7, s.visible_entries_tail);
    DA_FREE(FileTreeNode, &tree);
    PASS();
}

TEST ctrl_d_moves_half_page_down(void) {
    /* Tree: 10 nodes (root + f00..f08), all visible. lines=6, half=3.
     * Init: head=0, sel=0, tail=5 (six visible: 0..5).
     * Ctrl-D iterates 3 times; each slides window down by 1.
     * Final: head=3, tail=8, sel=3. */
    FileTree tree = {0};
    build_flat_tree(&tree, 9);
    AppState s = {0};
    init_app_state(&s, &tree, 6);
    handle_key(&s, &tree, 4 /* Ctrl-D */, 6);
    ASSERT_EQ(3, s.visible_entries_head);
    ASSERT_EQ(8, s.visible_entries_tail);
    ASSERT_EQ(3, s.selected_entry);
    DA_FREE(FileTreeNode, &tree);
    PASS();
}

TEST ctrl_u_moves_half_page_up(void) {
    /* Tree: 10 nodes. lines=6, half=3.
     * After 1st Ctrl-D (3 slides): head=3, tail=8, sel=3.
     * After 2nd Ctrl-D: 1st iter slides to head=4/tail=9/sel=4; 2nd & 3rd
     * iters are at-end (no slide), only sel advances via next().
     * Pre-Ctrl-U state: head=4, tail=9, sel=6.
     * Ctrl-U iterates 3 times; each slides window up by 1 and sel back by 1.
     * Final: head=1, tail=6, sel=3. */
    FileTree tree = {0};
    build_flat_tree(&tree, 9);
    AppState s = {0};
    init_app_state(&s, &tree, 6);
    handle_key(&s, &tree, 4, 6);
    handle_key(&s, &tree, 4, 6);
    ASSERT_EQ(4, s.visible_entries_head);
    ASSERT_EQ(9, s.visible_entries_tail);
    ASSERT_EQ(6, s.selected_entry);
    handle_key(&s, &tree, 21 /* Ctrl-U */, 6);
    ASSERT_EQ(1, s.visible_entries_head);
    ASSERT_EQ(6, s.visible_entries_tail);
    ASSERT_EQ(3, s.selected_entry);
    DA_FREE(FileTreeNode, &tree);
    PASS();
}

TEST resize_recomputes_tail(void) {
    /* Tree: 10 nodes. lines=4 → tail=3 (visible: 0,1,2,3).
     * KEY_RESIZE with lines=8 → tail=7 (visible: 0..7).
     * sel stays at 0 since 0 <= new tail (no clamp). */
    FileTree tree = {0};
    build_flat_tree(&tree, 9);
    AppState s = {0};
    init_app_state(&s, &tree, 4);
    ASSERT_EQ(0, s.visible_entries_head);
    ASSERT_EQ(3, s.visible_entries_tail);
    ASSERT_EQ(0, s.selected_entry);
    handle_key(&s, &tree, KEY_RESIZE, 8);
    ASSERT_EQ(0, s.visible_entries_head);
    ASSERT_EQ(7, s.visible_entries_tail);
    ASSERT_EQ(0, s.selected_entry);
    DA_FREE(FileTreeNode, &tree);
    PASS();
}

SUITE(tui_suite) {
    RUN_TEST(init_app_state_with_window_smaller_than_tree);
    RUN_TEST(init_app_state_with_window_larger_than_tree);
    RUN_TEST(q_returns_one);
    RUN_TEST(j_moves_selection_down);
    RUN_TEST(key_down_arrow_same_as_j);
    RUN_TEST(j_at_window_bottom_slides_window);
    RUN_TEST(j_at_tree_end_wraps_to_top);
    RUN_TEST(k_moves_selection_up);
    RUN_TEST(key_up_arrow_same_as_k);
    RUN_TEST(h_on_directory_collapses);
    RUN_TEST(l_on_collapsed_directory_expands);
    RUN_TEST(h_on_file_is_noop);
    RUN_TEST(g_resets_to_top);
    RUN_TEST(G_jumps_to_bottom);
    RUN_TEST(ctrl_d_moves_half_page_down);
    RUN_TEST(ctrl_u_moves_half_page_up);
    RUN_TEST(resize_recomputes_tail);
}
