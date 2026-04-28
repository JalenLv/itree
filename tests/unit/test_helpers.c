#include "greatest.h"
#include "helpers.h"
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

typedef struct { DA_FIELDS(int); } IntArray;
typedef struct { DQ_FIELDS(int); } IntDeque;

TEST da_push_pop_basic(void) {
    IntArray a = {0};
    DA_PUSH(int, &a, 1);
    DA_PUSH(int, &a, 2);
    DA_PUSH(int, &a, 3);
    ASSERT_EQ(3, a.count);
    ASSERT_EQ(3, DA_GET(int, &a, 2));
    ASSERT_EQ(3, DA_POP(int, &a));
    ASSERT_EQ(2, a.count);
    DA_FREE(int, &a);
    PASS();
}

TEST da_remove_keeps_order(void) {
    IntArray a = {0};
    for (int i = 0; i < 5; ++i) DA_PUSH(int, &a, i);
    DA_REMOVE(int, &a, 2);
    ASSERT_EQ(4, a.count);
    ASSERT_EQ(0, DA_GET(int, &a, 0));
    ASSERT_EQ(1, DA_GET(int, &a, 1));
    ASSERT_EQ(3, DA_GET(int, &a, 2));
    ASSERT_EQ(4, DA_GET(int, &a, 3));
    DA_FREE(int, &a);
    PASS();
}

TEST da_remove_range_keeps_order(void) {
    IntArray a = {0};
    for (int i = 0; i < 6; ++i) DA_PUSH(int, &a, i);
    DA_REMOVE_RANGE(int, &a, 1, 4);
    ASSERT_EQ(3, a.count);
    ASSERT_EQ(0, DA_GET(int, &a, 0));
    ASSERT_EQ(4, DA_GET(int, &a, 1));
    ASSERT_EQ(5, DA_GET(int, &a, 2));
    DA_FREE(int, &a);
    PASS();
}

TEST da_insert_merges_two_arrays(void) {
    IntArray a = {0};
    IntArray b = {0};
    for (int i = 0; i < 3; ++i) DA_PUSH(int, &a, i * 10);
    for (int i = 0; i < 2; ++i) DA_PUSH(int, &b, 99 + i);
    DA_INSERT(int, &a, 1, &b);
    ASSERT_EQ(5, a.count);
    ASSERT_EQ(0,  DA_GET(int, &a, 0));
    ASSERT_EQ(99, DA_GET(int, &a, 1));
    ASSERT_EQ(100,DA_GET(int, &a, 2));
    ASSERT_EQ(10, DA_GET(int, &a, 3));
    ASSERT_EQ(20, DA_GET(int, &a, 4));
    DA_FREE(int, &a);
    DA_FREE(int, &b);
    PASS();
}

static int int_cmp(const void *a, const void *b) {
    int ai = *(const int *)a, bi = *(const int *)b;
    return (ai > bi) - (ai < bi);
}

TEST da_sort_full(void) {
    IntArray a = {0};
    int input[] = {3, 1, 4, 1, 5, 9, 2, 6};
    for (size_t i = 0; i < sizeof(input)/sizeof(input[0]); ++i) DA_PUSH(int, &a, input[i]);
    DA_SORT(int, &a, int_cmp);
    int expected[] = {1, 1, 2, 3, 4, 5, 6, 9};
    for (int i = 0; i < a.count; ++i) ASSERT_EQ(expected[i], DA_GET(int, &a, i));
    DA_FREE(int, &a);
    PASS();
}

TEST da_sort_range_only_sorts_subrange(void) {
    IntArray a = {0};
    int input[] = {9, 5, 3, 1, 7};
    for (size_t i = 0; i < sizeof(input)/sizeof(input[0]); ++i) DA_PUSH(int, &a, input[i]);
    DA_SORT_RANGE(int, &a, 1, 4, int_cmp);
    int expected[] = {9, 1, 3, 5, 7};
    for (int i = 0; i < a.count; ++i) ASSERT_EQ(expected[i], DA_GET(int, &a, i));
    DA_FREE(int, &a);
    PASS();
}

TEST dq_back_front_basic(void) {
    IntDeque q = {0};
    DQ_PUSH_BACK(int, &q, 2);
    DQ_PUSH_BACK(int, &q, 3);
    DQ_PUSH_FRONT(int, &q, 1);
    ASSERT_EQ(3, (int)q.count);
    ASSERT_EQ(1, DQ_FRONT(int, &q));
    ASSERT_EQ(3, DQ_BACK(int, &q));
    ASSERT_EQ(2, DQ_GET(int, &q, 1));
    ASSERT_EQ(1, DQ_POP_FRONT(int, &q));
    ASSERT_EQ(3, DQ_POP_BACK(int, &q));
    ASSERT_EQ(1, (int)q.count);
    ASSERT_EQ(2, DQ_FRONT(int, &q));
    free(q.items);
    PASS();
}

TEST dq_resize_preserves_order(void) {
    IntDeque q = {0};
    for (size_t i = 0; i < 20; ++i) DQ_PUSH_BACK(int, &q, (int)i);
    for (size_t i = 0; i < 20; ++i) ASSERT_EQ((int)i, DQ_GET(int, &q, i));
    free(q.items);
    PASS();
}

TEST concat_basic(void) {
    char *s = concat("hello", ", ", "world");
    ASSERT_STR_EQ("hello, world", s);
    free(s);
    PASS();
}

TEST concat_single_arg(void) {
    char *s = concat_NULL("solo", NULL);
    ASSERT_STR_EQ("solo", s);
    free(s);
    PASS();
}

TEST concat_null_first_returns_null(void) {
    char *s = concat_NULL(NULL);
    ASSERT_EQ(NULL, s);
    PASS();
}

#include "argparse.h"
TEST open_io_default_is_stdout(void) {
    Args args = {0};
    args.output_file = NULL;
    FILE *out = NULL;
    ASSERT_EQ(0, open_io(&args, &out));
    ASSERT_EQ(stdout, out);
    close_io(out);
    PASS();
}

TEST open_io_writes_to_named_file(void) {
    char tmp[] = "/tmp/itree_test_open_io_XXXXXX";
    int fd = mkstemp(tmp);
    ASSERT(fd >= 0);
    close(fd);

    Args args = {0};
    args.output_file = tmp;
    FILE *out = NULL;
    ASSERT_EQ(0, open_io(&args, &out));
    ASSERT(out != NULL);
    ASSERT(out != stdout);
    fputs("hi", out);
    close_io(out);

    FILE *r = fopen(tmp, "r");
    char buf[8] = {0};
    fread(buf, 1, sizeof(buf) - 1, r);
    fclose(r);
    ASSERT_STR_EQ("hi", buf);
    unlink(tmp);
    PASS();
}

#include <unistd.h>

SUITE(helpers_suite) {
    RUN_TEST(da_push_pop_basic);
    RUN_TEST(da_remove_keeps_order);
    RUN_TEST(da_remove_range_keeps_order);
    RUN_TEST(da_insert_merges_two_arrays);
    RUN_TEST(da_sort_full);
    RUN_TEST(da_sort_range_only_sorts_subrange);
    RUN_TEST(dq_back_front_basic);
    RUN_TEST(dq_resize_preserves_order);
    RUN_TEST(concat_basic);
    RUN_TEST(concat_single_arg);
    RUN_TEST(concat_null_first_returns_null);
    RUN_TEST(open_io_default_is_stdout);
    RUN_TEST(open_io_writes_to_named_file);
}
