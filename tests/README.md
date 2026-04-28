# itree test suite

Three layers, run together with `make test` from the project root:

- **Unit tests** (`tests/unit/`) — link `src/*.c` (minus `main.c`) against
  [greatest](https://github.com/silentbicycle/greatest) v1.5.0 and exercise
  individual functions. Built into `tests/unit/runner` (gitignored).
- **Integration tests** (`tests/integration/run.sh`) — run the built `./itree`
  binary with `--no-tui` against fixtures in `tests/fixtures/` and diff
  stdout against goldens in `tests/golden/`.
- **TUI dump tests** (`tests/dumps/`) — drive `./itree` (TUI mode) inside a
  pseudo-terminal via `pexpect`, render output through `pyte` (a VT
  emulator), and diff the resulting 24×80 cell grid against goldens in
  `tests/dumps/golden/`. Requires `pyte` and `pexpect`; if either is
  missing the runner exits 0 with a warning so the rest of the suite still
  runs.

## Running

```bash
./configure
make build       # produces ./itree
make test        # all three layers

# Subsets
make test-unit
make test-integration
make test-dumps

# Regenerate dump goldens after intentional UI changes
./tests/dumps/runner.py --update
```

To install the Python deps for dump tests:

```bash
pip install pyte pexpect
```

## How to add tests

### A new unit test

1. Pick the relevant `tests/unit/test_*.c` file (or create one).
2. Add a `TEST my_test_name(void) { ... PASS(); }` function using greatest's
   `ASSERT_*` macros.
3. Add `RUN_TEST(my_test_name);` to that file's `SUITE(...)` block.
4. If you created a new file with a new suite, add `extern SUITE_EXTERN(...)`
   and `RUN_SUITE(...)` lines in `tests/unit/runner.c`.
5. `make test-unit` to verify.

Useful helpers in `tests/unit/test_util.h`:

- `synth_push(tree, type, name, depth, collapsed, target)` — append a node
  to a `FileTree` without touching the filesystem.

### A new integration (CLI) test

1. If you need a new fixture, drop it under `tests/fixtures/<name>/`.
2. Seed the golden:
   `./itree --no-tui tests/fixtures/<name> > tests/golden/<name>.txt`
3. Add a line to `tests/integration/run.sh`:
   `expect_match <name> "$GOLDEN/<name>.txt" "$ITREE" --no-tui "$FIXTURES/<name>"`
4. `make test-integration` to verify.

Use `expect_fail <name> "$ITREE" ...` for cases that should exit non-zero.

### A new TUI dump test

1. Create `tests/dumps/cases/NN_short_name.py` with:

   ```python
   FIXTURE = "tests/fixtures/<name>"
   EXTRA_ARGV = ["-a"]            # optional, e.g. for flag-driven tests
   KEYS = ["j", "j", "h"]          # keystrokes; q is appended automatically
   GOLDEN = "NN_short_name.dump"
   ```

2. Generate the golden once:
   `./tests/dumps/runner.py --update`
3. Inspect `tests/dumps/golden/NN_short_name.dump` to confirm the screen
   matches your expectation, then commit it.
4. `make test-dumps` to verify it reproduces.

## Layout

```
tests/
├── README.md
├── greatest.h                  # vendored, v1.5.0
├── unit/
│   ├── runner.c                # GREATEST_MAIN_DEFS + suite list
│   ├── test_argparse.c
│   ├── test_file_tree.c
│   ├── test_draw_tree.c
│   ├── test_helpers.c
│   ├── test_tui.c
│   └── test_util.h             # synthesis helpers
├── fixtures/                   # on-disk test data
├── golden/                     # expected --no-tui output
├── integration/
│   └── run.sh
└── dumps/
    ├── runner.py               # pty + pyte
    ├── cases/                  # one .py per case
    └── golden/                 # 24×80 cell-grid dumps
```
