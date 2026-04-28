#include "greatest.h"

GREATEST_MAIN_DEFS();

extern SUITE_EXTERN(helpers_suite);
extern SUITE_EXTERN(argparse_suite);
extern SUITE_EXTERN(file_tree_suite);
extern SUITE_EXTERN(draw_tree_suite);
extern SUITE_EXTERN(tui_suite);

int main(int argc, char **argv) {
    GREATEST_MAIN_BEGIN();
    RUN_SUITE(argparse_suite);
    RUN_SUITE(helpers_suite);
    RUN_SUITE(file_tree_suite);
    RUN_SUITE(draw_tree_suite);
    RUN_SUITE(tui_suite);
    GREATEST_MAIN_END();
}
