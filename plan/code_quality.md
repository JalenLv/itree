| Check | Priority | Location | Issue | Suggested Solution |
|---|---|---|---|---|
| Checked | P0 | src/file_tree.c:49 | Unsupported file types print “Skipping” but return failure, aborting the entire scan. | Change this path to continue; treat sockets, FIFOs, devices, etc. as non-fatal skipped entries. |
| Checked | P0 | src/file_tree.c:83 | Early returns inside walk() skip closedir(d). | Use a single cleanup: label or return-code variable so every error path closes the directory handle. |
| Checked | P0 | src/file_tree.c:56 | FileTreeNode node; is uninitialized, so fields like target can contain stale stack data. | Use FileTreeNode node = {0}; before setting fields. |
| Checked | P0 | src/file_tree.c:36 | If lstat() fails for DT_UNKNOWN, the code can continue with no valid type selected. | On lstat() failure, warn and continue, or fail cleanly after closing DIR *d. |
|  | P0 | include/helpers.h:33 | DA_PUSH overwrites items with realloc() directly and never checks failure. | Replace with a reserve helper/macro that uses a temporary pointer and returns failure to callers. |
| Checked | P0 | src/file_tree.c:20 | snprintf(fullpath, ...) can silently truncate deep paths. | Check the return value; if it is negative or >= sizeof(fullpath), report “path too long” and skip/fail cleanly. |
| See arena.md | P1  | src/file_tree.c:73 | Symlink targets are stored in fixed target[256]; long targets can be truncated. | Detect readlink() filling the buffer, or move name/target to dynamically allocated strings. |
| Checked | P1 | src/file_tree.c:97 | Root path copy can silently truncate before basename(). | Check snprintf return value before calling basename(). |
| Checked | P1 | src/tui.c:120 | TUI output uses stdout; if final tree output is piped, curses escape codes can pollute stdout. | Open /dev/tty for TUI output too, and pass that stream to newterm(). |
| Checked | P1 | src/tui.c:138 | Error paths after newterm() return without endwin(), delscreen(), or closing the TTY. | Use a single cleanup path in run_tui() for all exits after curses initialization. |
| Checked | P1 | src/tui.c:85 | Non-link wide-character conversion ignores mbstowcs() failure. | Check the return value like the link branch does; fallback to narrow printw() on failure. |
| ? | P1 | src/draw_tree.c:64 | Output write failures are ignored. Broken pipe or disk-full errors can still return success. | Check fprintf/fputs results or call ferror(output) before returning. |
|  | P1 | include/helpers.h:61 | DA_INSERT has the same unchecked realloc() problem as DA_PUSH. | Apply the same reserve-helper approach, or remove unused macros until needed. |
|  | P1 | include/helpers.h:84 | Deque resize uses unchecked malloc(). | Add allocation failure handling or delete unused deque macros. |
| Think About It | P2 | include/helpers.h:18 | Dynamic array count and capacity are int, causing signed/size conversion warnings. | Use size_t for counts/capacities and update index handling consistently. |
|  | P2 | include/helpers.h:43 | DA_GET triggers strict compile warnings for struct casts and has macro-evaluation risks. | Prefer typed inline/helper functions for FileTree, or simplify the macro to avoid unnecessary casts. |
| Checked | P2 | include/draw_tree.h:9 | Header uses FILE * but does not directly include <stdio.h>. | Add #include <stdio.h> in draw_tree.h so the header is self-contained. |
| Checked | P2 | include/argparse.h:7 | Comment mentions input_file, but the struct has path. | Update the comment to match the actual fields. |
| ? | P2 | src/argparse.c:18 | getopt_long() uses global parser state, which makes repeated unit tests awkward. | Reset optind = 1 at the start of parse_args() if this function will be tested directly. |

