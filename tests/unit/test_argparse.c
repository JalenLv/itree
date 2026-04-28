#include "greatest.h"
#include "argparse.h"
#include <getopt.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

/*
 * getopt_long uses global state (optind, optreset, opterr). Each test must
 * reset it before calling parse_args, or option parsing leaks across tests.
 * Also silence getopt's own stderr complaints so error-path tests stay quiet.
 */
#if defined(__linux__) || defined(__GLIBC__)
#define RESET_GETOPT() do { optind = 0; opterr = 0; } while (0)
#else
extern int optreset;
#define RESET_GETOPT() do { optind = 1; optreset = 1; opterr = 0; } while (0)
#endif

static int call_parse(Args *args, int argc, char *argv[]) {
    RESET_GETOPT();
    /* Redirect stderr to /dev/null for the duration of the call so error
     * cases don't pollute test output. */
    fflush(stderr);
    int saved = dup(fileno(stderr));
    FILE *null = freopen("/dev/null", "w", stderr);
    (void)null;
    int rc = parse_args(argc, argv, args);
    fflush(stderr);
    dup2(saved, fileno(stderr));
    close(saved);
    return rc;
}

TEST defaults_when_no_args(void) {
    Args args = {0};
    char *argv[] = {"itree"};
    ASSERT_EQ(0, call_parse(&args, 1, argv));
    ASSERT_STR_EQ(".", args.path);
    ASSERT_EQ(NULL, args.output_file);
    ASSERT_EQ(0, args.show_help);
    ASSERT_EQ(0, args.show_hidden);
    ASSERT_EQ(0, args.no_tui);
    PASS();
}

TEST short_o_sets_output(void) {
    Args args = {0};
    char *argv[] = {"itree", "-o", "out.txt"};
    ASSERT_EQ(0, call_parse(&args, 3, argv));
    ASSERT_STR_EQ("out.txt", args.output_file);
    PASS();
}

TEST long_output_sets_output(void) {
    Args args = {0};
    char *argv[] = {"itree", "--output", "out.txt"};
    ASSERT_EQ(0, call_parse(&args, 3, argv));
    ASSERT_STR_EQ("out.txt", args.output_file);
    PASS();
}

TEST short_h_sets_show_help(void) {
    Args args = {0};
    char *argv[] = {"itree", "-h"};
    ASSERT_EQ(0, call_parse(&args, 2, argv));
    ASSERT_EQ(1, args.show_help);
    PASS();
}

TEST long_help_sets_show_help(void) {
    Args args = {0};
    char *argv[] = {"itree", "--help"};
    ASSERT_EQ(0, call_parse(&args, 2, argv));
    ASSERT_EQ(1, args.show_help);
    PASS();
}

TEST short_a_sets_show_hidden(void) {
    Args args = {0};
    char *argv[] = {"itree", "-a"};
    ASSERT_EQ(0, call_parse(&args, 2, argv));
    ASSERT_EQ(1, args.show_hidden);
    PASS();
}

TEST long_show_hidden_sets_show_hidden(void) {
    Args args = {0};
    char *argv[] = {"itree", "--show-hidden"};
    ASSERT_EQ(0, call_parse(&args, 2, argv));
    ASSERT_EQ(1, args.show_hidden);
    PASS();
}

TEST long_no_tui_sets_no_tui(void) {
    Args args = {0};
    char *argv[] = {"itree", "--no-tui"};
    ASSERT_EQ(0, call_parse(&args, 2, argv));
    ASSERT_EQ(1, args.no_tui);
    PASS();
}

TEST positional_path_sets_path(void) {
    Args args = {0};
    char *argv[] = {"itree", "/tmp/some/dir"};
    ASSERT_EQ(0, call_parse(&args, 2, argv));
    ASSERT_STR_EQ("/tmp/some/dir", args.path);
    PASS();
}

TEST flags_with_path_combined(void) {
    Args args = {0};
    char *argv[] = {"itree", "-a", "--no-tui", "-o", "out.txt", "tests/fixtures/flat"};
    ASSERT_EQ(0, call_parse(&args, 6, argv));
    ASSERT_EQ(1, args.show_hidden);
    ASSERT_EQ(1, args.no_tui);
    ASSERT_STR_EQ("out.txt", args.output_file);
    ASSERT_STR_EQ("tests/fixtures/flat", args.path);
    PASS();
}

TEST too_many_paths_errors(void) {
    Args args = {0};
    char *argv[] = {"itree", "a", "b"};
    ASSERT(call_parse(&args, 3, argv) != 0);
    PASS();
}

TEST unknown_option_errors(void) {
    Args args = {0};
    char *argv[] = {"itree", "--this-is-not-real"};
    ASSERT(call_parse(&args, 2, argv) != 0);
    PASS();
}

SUITE(argparse_suite) {
    RUN_TEST(defaults_when_no_args);
    RUN_TEST(short_o_sets_output);
    RUN_TEST(long_output_sets_output);
    RUN_TEST(short_h_sets_show_help);
    RUN_TEST(long_help_sets_show_help);
    RUN_TEST(short_a_sets_show_hidden);
    RUN_TEST(long_show_hidden_sets_show_hidden);
    RUN_TEST(long_no_tui_sets_no_tui);
    RUN_TEST(positional_path_sets_path);
    RUN_TEST(flags_with_path_combined);
    RUN_TEST(too_many_paths_errors);
    RUN_TEST(unknown_option_errors);
}
