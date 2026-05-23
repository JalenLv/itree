#include "greatest.h"
#include "helpers.h"
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

typedef struct { DA_FIELDS(int); } IntArray;
DA_DEFINE(IntArray, int);
typedef struct { DQ_FIELDS(int); } IntDeque;

TEST da_push_pop_basic(void) {
    IntArray a = {0};
    IntArray_push(&a, &(int){1});
    IntArray_push(&a, &(int){2});
    IntArray_push(&a, &(int){3});
    ASSERT_EQ(3, (int)a.count);
    int got;
    IntArray_get(&a, 2, &got);
    ASSERT_EQ(3, got);
    IntArray_pop(&a, &got);
    ASSERT_EQ(3, got);
    ASSERT_EQ(2, (int)a.count);
    IntArray_free(&a);
    PASS();
}

TEST da_remove_keeps_order(void) {
    IntArray a = {0};
    for (int i = 0; i < 5; ++i) IntArray_push(&a, &i);
    IntArray_remove(&a, 2);
    ASSERT_EQ(4, (int)a.count);
    int got;
    IntArray_get(&a, 0, &got); ASSERT_EQ(0, got);
    IntArray_get(&a, 1, &got); ASSERT_EQ(1, got);
    IntArray_get(&a, 2, &got); ASSERT_EQ(3, got);
    IntArray_get(&a, 3, &got); ASSERT_EQ(4, got);
    IntArray_free(&a);
    PASS();
}

TEST da_remove_range_keeps_order(void) {
    IntArray a = {0};
    for (int i = 0; i < 6; ++i) IntArray_push(&a, &i);
    IntArray_remove_range(&a, 1, 4);
    ASSERT_EQ(3, (int)a.count);
    int got;
    IntArray_get(&a, 0, &got); ASSERT_EQ(0, got);
    IntArray_get(&a, 1, &got); ASSERT_EQ(4, got);
    IntArray_get(&a, 2, &got); ASSERT_EQ(5, got);
    IntArray_free(&a);
    PASS();
}

TEST da_insert_merges_two_arrays(void) {
    IntArray a = {0};
    IntArray b = {0};
    for (int i = 0; i < 3; ++i) { int v = i * 10; IntArray_push(&a, &v); }
    for (int i = 0; i < 2; ++i) { int v = 99 + i; IntArray_push(&b, &v); }
    IntArray_insert(&a, &b, 1);
    ASSERT_EQ(5, (int)a.count);
    int got;
    IntArray_get(&a, 0, &got); ASSERT_EQ(0,   got);
    IntArray_get(&a, 1, &got); ASSERT_EQ(99,  got);
    IntArray_get(&a, 2, &got); ASSERT_EQ(100, got);
    IntArray_get(&a, 3, &got); ASSERT_EQ(10,  got);
    IntArray_get(&a, 4, &got); ASSERT_EQ(20,  got);
    IntArray_free(&a);
    IntArray_free(&b);
    PASS();
}

static int int_cmp(const void *a, const void *b) {
    int ai = *(const int *)a, bi = *(const int *)b;
    return (ai > bi) - (ai < bi);
}

TEST da_sort_full(void) {
    IntArray a = {0};
    int input[] = {3, 1, 4, 1, 5, 9, 2, 6};
    for (size_t i = 0; i < sizeof(input)/sizeof(input[0]); ++i) IntArray_push(&a, &input[i]);
    IntArray_sort(&a, int_cmp);
    int expected[] = {1, 1, 2, 3, 4, 5, 6, 9};
    for (size_t i = 0; i < a.count; ++i) {
        int got;
        IntArray_get(&a, i, &got);
        ASSERT_EQ(expected[i], got);
    }
    IntArray_free(&a);
    PASS();
}

TEST da_sort_range_only_sorts_subrange(void) {
    IntArray a = {0};
    int input[] = {9, 5, 3, 1, 7};
    for (size_t i = 0; i < sizeof(input)/sizeof(input[0]); ++i) IntArray_push(&a, &input[i]);
    IntArray_sort_range(&a, 1, 4, int_cmp);
    int expected[] = {9, 1, 3, 5, 7};
    for (size_t i = 0; i < a.count; ++i) {
        int got;
        IntArray_get(&a, i, &got);
        ASSERT_EQ(expected[i], got);
    }
    IntArray_free(&a);
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
