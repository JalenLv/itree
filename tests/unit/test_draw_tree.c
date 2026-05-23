#include "greatest.h"
#include "draw_tree.h"
#include "file_tree.h"
#include "argparse.h"
#include "test_util.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Read an entire file into a heap-allocated buffer; caller frees. */
static char *slurp(const char *path) {
    FILE *f = fopen(path, "rb");
    if (!f) return NULL;
    fseek(f, 0, SEEK_END);
    long sz = ftell(f);
    fseek(f, 0, SEEK_SET);
    char *buf = malloc((size_t)sz + 1);
    if (!buf) { fclose(f); return NULL; }
    fread(buf, 1, (size_t)sz, f);
    buf[sz] = '\0';
    fclose(f);
    return buf;
}

/* Build a tree from a fixture, draw it to a memory buffer, diff against the
 * matching golden file. Returns 0 on match. */
static int draw_and_diff(const char *fixture_path, int show_hidden, const char *golden_path,
                        char *errbuf, size_t errbuf_sz) {
    FileTree tree = {0};
    Args args = {0};
    args.path = (char *)fixture_path;
    args.show_hidden = show_hidden;
    if (create_file_tree_from_path(&tree, &args) != 0) {
        snprintf(errbuf, errbuf_sz, "create_file_tree_from_path failed");
        return 1;
    }

    char buf[8192] = {0};
    FILE *out = fmemopen(buf, sizeof(buf), "w");
    if (!out) {
        FileTree_free(&tree);
        snprintf(errbuf, errbuf_sz, "fmemopen failed");
        return 1;
    }
    int rc = draw_tree(&tree, out);
    fclose(out);
    FileTree_free(&tree);
    if (rc != 0) {
        snprintf(errbuf, errbuf_sz, "draw_tree returned %d", rc);
        return 1;
    }

    char *expected = slurp(golden_path);
    if (!expected) {
        snprintf(errbuf, errbuf_sz, "could not read golden %s", golden_path);
        return 1;
    }
    int diff = strcmp(buf, expected);
    if (diff != 0) {
        snprintf(errbuf, errbuf_sz, "output mismatch\nexpected:\n%s\ngot:\n%s", expected, buf);
    }
    free(expected);
    return diff;
}

#define ASSERT_DRAW_MATCHES(fixture, hidden, golden) do {                                  \
    char err[4096] = {0};                                                                  \
    int diff = draw_and_diff((fixture), (hidden), (golden), err, sizeof(err));             \
    if (diff != 0) FAILm(err);                                                             \
} while (0)

TEST flat_matches_golden(void) {
    ASSERT_DRAW_MATCHES("tests/fixtures/flat", 0, "tests/golden/flat.txt");
    PASS();
}

TEST nested_matches_golden(void) {
    ASSERT_DRAW_MATCHES("tests/fixtures/nested", 0, "tests/golden/nested.txt");
    PASS();
}

TEST with_hidden_no_a_matches_golden(void) {
    ASSERT_DRAW_MATCHES("tests/fixtures/with_hidden", 0, "tests/golden/with_hidden.no_a.txt");
    PASS();
}

TEST with_hidden_with_a_matches_golden(void) {
    ASSERT_DRAW_MATCHES("tests/fixtures/with_hidden", 1, "tests/golden/with_hidden.with_a.txt");
    PASS();
}

TEST mixed_case_matches_golden(void) {
    ASSERT_DRAW_MATCHES("tests/fixtures/mixed_case", 0, "tests/golden/mixed_case.txt");
    PASS();
}

TEST with_link_matches_golden(void) {
    ASSERT_DRAW_MATCHES("tests/fixtures/with_link", 0, "tests/golden/with_link.txt");
    PASS();
}

TEST collapsed_directory_hides_its_children(void) {
    /* Build a synthetic tree where one dir is collapsed; assert its children
     * never appear in the output. */
    FileTree tree = {0};
    synth_push(&tree, DIRECTORY_NODE, "root",   0, 0, NULL);
    synth_push(&tree, DIRECTORY_NODE, "shown",  1, 0, NULL);
    synth_push(&tree, FILE_NODE,      "shown_child", 2, 0, NULL);
    synth_push(&tree, DIRECTORY_NODE, "collapsed", 1, 1 /* collapsed */, NULL);
    synth_push(&tree, FILE_NODE,      "buried_a",   2, 0, NULL);
    synth_push(&tree, FILE_NODE,      "buried_b",   2, 0, NULL);
    synth_push(&tree, FILE_NODE,      "tail",       1, 0, NULL);

    char buf[1024] = {0};
    FILE *out = fmemopen(buf, sizeof(buf), "w");
    ASSERT(out != NULL);
    ASSERT_EQ(0, draw_tree(&tree, out));
    fclose(out);

    ASSERT(strstr(buf, "shown_child") != NULL);
    ASSERT(strstr(buf, "collapsed")  != NULL);
    ASSERT(strstr(buf, "tail")       != NULL);
    ASSERT(strstr(buf, "buried_a")   == NULL);
    ASSERT(strstr(buf, "buried_b")   == NULL);

    FileTree_free(&tree);
    PASS();
}

SUITE(draw_tree_suite) {
    RUN_TEST(flat_matches_golden);
    RUN_TEST(nested_matches_golden);
    RUN_TEST(with_hidden_no_a_matches_golden);
    RUN_TEST(with_hidden_with_a_matches_golden);
    RUN_TEST(mixed_case_matches_golden);
    RUN_TEST(with_link_matches_golden);
    RUN_TEST(collapsed_directory_hides_its_children);
}
