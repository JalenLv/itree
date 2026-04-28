#include "greatest.h"
#include "file_tree.h"
#include "argparse.h"
#include "test_util.h"
#include <string.h>
#include <stdio.h>
#include <unistd.h>

/* ---------------------------------------------------------------------------
 * create_file_tree_from_path against on-disk fixtures
 * ------------------------------------------------------------------------ */

static Args make_args(char *path, int show_hidden) {
    Args args = {0};
    args.path = path;
    args.show_hidden = show_hidden;
    return args;
}

TEST flat_fixture_count_and_order(void) {
    FileTree tree = {0};
    Args args = make_args("tests/fixtures/flat", 0);
    ASSERT_EQ(0, create_file_tree_from_path(&tree, &args));
    ASSERT_EQ(5, tree.count); /* root + 4 files */
    ASSERT_STR_EQ("flat", tree.items[0].name);
    ASSERT_EQ(DIRECTORY_NODE, tree.items[0].type);
    ASSERT_EQ(0, tree.items[0].depth);
    ASSERT_STR_EQ("a.txt", tree.items[1].name);
    ASSERT_STR_EQ("b.txt", tree.items[2].name);
    ASSERT_STR_EQ("c.txt", tree.items[3].name);
    ASSERT_STR_EQ("d.txt", tree.items[4].name);
    for (int i = 1; i < 5; ++i) {
        ASSERT_EQ(FILE_NODE, tree.items[i].type);
        ASSERT_EQ(1, tree.items[i].depth);
    }
    DA_FREE(FileTreeNode, &tree);
    PASS();
}

TEST mixed_case_sort_is_case_insensitive(void) {
    FileTree tree = {0};
    Args args = make_args("tests/fixtures/mixed_case", 0);
    ASSERT_EQ(0, create_file_tree_from_path(&tree, &args));
    /* Sorted case-insensitively: Apple, banana, Cherry, date */
    ASSERT_STR_EQ("Apple",  tree.items[1].name);
    ASSERT_STR_EQ("banana", tree.items[2].name);
    ASSERT_STR_EQ("Cherry", tree.items[3].name);
    ASSERT_STR_EQ("date",   tree.items[4].name);
    DA_FREE(FileTreeNode, &tree);
    PASS();
}

TEST hidden_excluded_by_default(void) {
    FileTree tree = {0};
    Args args = make_args("tests/fixtures/with_hidden", 0);
    ASSERT_EQ(0, create_file_tree_from_path(&tree, &args));
    /* Root + beta + delta only (no .alpha, no .gamma) */
    ASSERT_EQ(3, tree.count);
    ASSERT_STR_EQ("beta",  tree.items[1].name);
    ASSERT_STR_EQ("delta", tree.items[2].name);
    DA_FREE(FileTreeNode, &tree);
    PASS();
}

TEST hidden_included_with_show_hidden(void) {
    FileTree tree = {0};
    Args args = make_args("tests/fixtures/with_hidden", 1);
    ASSERT_EQ(0, create_file_tree_from_path(&tree, &args));
    /* Root + .alpha + beta + delta + .gamma. Sort key strips leading dot. */
    ASSERT_EQ(5, tree.count);
    ASSERT_STR_EQ(".alpha", tree.items[1].name);
    ASSERT_STR_EQ("beta",   tree.items[2].name);
    ASSERT_STR_EQ("delta",  tree.items[3].name);
    ASSERT_STR_EQ(".gamma", tree.items[4].name);
    DA_FREE(FileTreeNode, &tree);
    PASS();
}

TEST nested_depths_are_correct(void) {
    FileTree tree = {0};
    Args args = make_args("tests/fixtures/nested", 0);
    ASSERT_EQ(0, create_file_tree_from_path(&tree, &args));
    /*
     * Expected DFS order after sort:
     *   nested(0) alpha(1) one.txt(2) two.txt(2) beta(1) nested_deep(2) leaf.txt(3) gamma.txt(1)
     */
    ASSERT_EQ(8, tree.count);
    ASSERT_STR_EQ("nested",      tree.items[0].name); ASSERT_EQ(0, tree.items[0].depth);
    ASSERT_STR_EQ("alpha",       tree.items[1].name); ASSERT_EQ(1, tree.items[1].depth);
    ASSERT_STR_EQ("one.txt",     tree.items[2].name); ASSERT_EQ(2, tree.items[2].depth);
    ASSERT_STR_EQ("two.txt",     tree.items[3].name); ASSERT_EQ(2, tree.items[3].depth);
    ASSERT_STR_EQ("beta",        tree.items[4].name); ASSERT_EQ(1, tree.items[4].depth);
    ASSERT_STR_EQ("nested_deep", tree.items[5].name); ASSERT_EQ(2, tree.items[5].depth);
    ASSERT_STR_EQ("leaf.txt",    tree.items[6].name); ASSERT_EQ(3, tree.items[6].depth);
    ASSERT_STR_EQ("gamma.txt",   tree.items[7].name); ASSERT_EQ(1, tree.items[7].depth);
    DA_FREE(FileTreeNode, &tree);
    PASS();
}

TEST with_link_records_target(void) {
    FileTree tree = {0};
    Args args = make_args("tests/fixtures/with_link", 0);
    ASSERT_EQ(0, create_file_tree_from_path(&tree, &args));
    ASSERT_EQ(3, tree.count);
    /* link_to_target sorts before target.txt alphabetically */
    ASSERT_STR_EQ("link_to_target", tree.items[1].name);
    ASSERT_EQ(LINK_NODE,             tree.items[1].type);
    ASSERT_STR_EQ("target.txt",      tree.items[1].target);
    ASSERT_STR_EQ("target.txt",      tree.items[2].name);
    ASSERT_EQ(FILE_NODE,             tree.items[2].type);
    DA_FREE(FileTreeNode, &tree);
    PASS();
}

TEST nonexistent_path_errors(void) {
    FileTree tree = {0};
    Args args = make_args("tests/fixtures/__definitely_does_not_exist__", 0);
    /* Suppress perror noise. */
    fflush(stderr);
    int saved = dup(fileno(stderr));
    FILE *null = freopen("/dev/null", "w", stderr); (void)null;
    int rc = create_file_tree_from_path(&tree, &args);
    fflush(stderr);
    dup2(saved, fileno(stderr));
    close(saved);

    ASSERT(rc != 0);
    /* On failure, tree is freed by create_file_tree_from_path (count back to 0) */
    ASSERT_EQ(0, tree.count);
    PASS();
}

/* ---------------------------------------------------------------------------
 * next() / prev() with synthetic trees
 * ------------------------------------------------------------------------ */

TEST next_advances_through_files(void) {
    FileTree tree = {0};
    synth_push(&tree, DIRECTORY_NODE, "root", 0, 0, NULL);
    synth_push(&tree, FILE_NODE,      "a",    1, 0, NULL);
    synth_push(&tree, FILE_NODE,      "b",    1, 0, NULL);
    synth_push(&tree, FILE_NODE,      "c",    1, 0, NULL);

    ASSERT_EQ(1, next(&tree, 0));
    ASSERT_EQ(2, next(&tree, 1));
    ASSERT_EQ(3, next(&tree, 2));
    ASSERT_EQ(0, next(&tree, 3)); /* wrap to first visible */

    DA_FREE(FileTreeNode, &tree);
    PASS();
}

TEST next_skips_children_of_collapsed_dir(void) {
    FileTree tree = {0};
    synth_push(&tree, DIRECTORY_NODE, "root",     0, 0, NULL);
    synth_push(&tree, DIRECTORY_NODE, "subdir",   1, 1 /* collapsed */, NULL);
    synth_push(&tree, FILE_NODE,      "hidden_a", 2, 0, NULL);
    synth_push(&tree, FILE_NODE,      "hidden_b", 2, 0, NULL);
    synth_push(&tree, FILE_NODE,      "after",    1, 0, NULL);

    /* From root, next is subdir */
    ASSERT_EQ(1, next(&tree, 0));
    /* From collapsed subdir, next jumps over its children to "after" */
    ASSERT_EQ(4, next(&tree, 1));
    /* After is last; wraps */
    ASSERT_EQ(0, next(&tree, 4));

    DA_FREE(FileTreeNode, &tree);
    PASS();
}

TEST prev_walks_back_to_zero(void) {
    FileTree tree = {0};
    synth_push(&tree, DIRECTORY_NODE, "root", 0, 0, NULL);
    synth_push(&tree, FILE_NODE,      "a",    1, 0, NULL);
    synth_push(&tree, FILE_NODE,      "b",    1, 0, NULL);

    ASSERT_EQ(1, prev(&tree, 2));
    ASSERT_EQ(0, prev(&tree, 1));
    /* prev(0) returns the last visible node */
    ASSERT_EQ(2, prev(&tree, 0));

    DA_FREE(FileTreeNode, &tree);
    PASS();
}

TEST prev_skips_collapsed_subtree(void) {
    FileTree tree = {0};
    synth_push(&tree, DIRECTORY_NODE, "root",   0, 0, NULL);
    synth_push(&tree, DIRECTORY_NODE, "sub",    1, 1, NULL);
    synth_push(&tree, FILE_NODE,      "buried", 2, 0, NULL);
    synth_push(&tree, FILE_NODE,      "after",  1, 0, NULL);

    /* From "after" (idx 3), prev should skip over "buried" (collapsed parent)
     * and return "sub" (idx 1). */
    ASSERT_EQ(1, prev(&tree, 3));

    DA_FREE(FileTreeNode, &tree);
    PASS();
}

TEST next_and_prev_invalid_index(void) {
    FileTree tree = {0};
    synth_push(&tree, DIRECTORY_NODE, "root", 0, 0, NULL);
    ASSERT_EQ(-1, next(&tree, 99));
    ASSERT_EQ(-1, prev(&tree, 99));
    DA_FREE(FileTreeNode, &tree);
    PASS();
}

SUITE(file_tree_suite) {
    RUN_TEST(flat_fixture_count_and_order);
    RUN_TEST(mixed_case_sort_is_case_insensitive);
    RUN_TEST(hidden_excluded_by_default);
    RUN_TEST(hidden_included_with_show_hidden);
    RUN_TEST(nested_depths_are_correct);
    RUN_TEST(with_link_records_target);
    RUN_TEST(nonexistent_path_errors);
    RUN_TEST(next_advances_through_files);
    RUN_TEST(next_skips_children_of_collapsed_dir);
    RUN_TEST(prev_walks_back_to_zero);
    RUN_TEST(prev_skips_collapsed_subtree);
    RUN_TEST(next_and_prev_invalid_index);
}
