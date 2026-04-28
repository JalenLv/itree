#!/usr/bin/env python3
"""TUI screen-dump tests for itree.

Spawns ``./itree`` inside a pseudo-terminal at a fixed 24x80 size, drives a
sequence of keystrokes, and feeds the output bytes through ``pyte`` (a VT
emulator) to obtain a deterministic cell-grid dump. The dump is compared
against a golden text file under ``tests/dumps/golden/``.

Each case file under ``tests/dumps/cases/`` defines three module-level
variables (plus one optional)::

    FIXTURE = "tests/fixtures/<name>"   # path passed to ./itree
    KEYS    = ["j", "j", ...]            # keystrokes; "q" appended automatically
    GOLDEN  = "<name>.dump"              # filename under tests/dumps/golden/
    EXTRA_ARGV = ["-a"]                  # optional: extra argv before FIXTURE

Run from the project root::

    ./tests/dumps/runner.py             # compare against goldens
    ./tests/dumps/runner.py --update    # regenerate goldens

If ``pyte`` or ``pexpect`` is not installed, the runner prints a warning and
exits 0 so the rest of the suite still runs in environments without Python
test deps.
"""
import os
import sys
import time
import runpy
import argparse

try:
    import pyte
    import pexpect
except ImportError as e:
    print(f"WARN: skipping TUI dump tests ({e}). "
          f"Install with: pip install pyte pexpect", file=sys.stderr)
    sys.exit(0)

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), os.pardir, os.pardir))
ITREE = os.environ.get("ITREE", os.path.join(ROOT, "itree"))
CASES_DIR = os.path.join(ROOT, "tests", "dumps", "cases")
GOLDEN_DIR = os.path.join(ROOT, "tests", "dumps", "golden")

ROWS, COLS = 24, 80


def settle(child, screen, stream, idle_ms=200, max_ms=2000):
    """Read output from `child` and feed it to `stream` until `idle_ms` of
    quiet, or `max_ms` overall. Strips ANSI escapes via pyte."""
    deadline = time.time() + max_ms / 1000.0
    last = time.time()
    while time.time() < deadline:
        try:
            buf = child.read_nonblocking(size=4096, timeout=0.05)
        except pexpect.TIMEOUT:
            if (time.time() - last) * 1000 > idle_ms:
                return
            continue
        except pexpect.EOF:
            return
        if buf:
            stream.feed(buf)
            last = time.time()


def render_dump(screen):
    """Return the screen content as one newline-separated string with trailing
    whitespace trimmed off each row."""
    return "\n".join(row.rstrip() for row in screen.display) + "\n"


def run_case(case_path):
    """Run a single case file. Returns (case_name, ok, message)."""
    name = os.path.splitext(os.path.basename(case_path))[0]
    spec = runpy.run_path(case_path)
    fixture = spec["FIXTURE"]
    keys = list(spec.get("KEYS", []))
    extra_argv = list(spec.get("EXTRA_ARGV", []))
    golden_name = spec["GOLDEN"]
    golden_path = os.path.join(GOLDEN_DIR, golden_name)

    env = os.environ.copy()
    env.update({
        "TERM": "xterm-256color",
        "LINES": str(ROWS),
        "COLUMNS": str(COLS),
        "LC_ALL": "C.UTF-8",
    })

    screen = pyte.Screen(COLS, ROWS)
    stream = pyte.ByteStream(screen)

    child = pexpect.spawn(ITREE, extra_argv + [fixture], dimensions=(ROWS, COLS),
                          env=env, encoding=None, timeout=2)
    try:
        settle(child, screen, stream)
        for k in keys:
            child.send(k.encode("utf-8") if isinstance(k, str) else k)
            settle(child, screen, stream)
        actual = render_dump(screen)
        # Quit cleanly so the binary tears down ncurses and exits.
        try:
            child.send(b"q")
            child.expect(pexpect.EOF, timeout=2)
        except (pexpect.TIMEOUT, OSError):
            pass
    finally:
        try:
            child.close(force=True)
        except Exception:
            pass

    if UPDATE:
        os.makedirs(GOLDEN_DIR, exist_ok=True)
        with open(golden_path, "w") as f:
            f.write(actual)
        return name, True, f"updated {golden_path}"

    if not os.path.exists(golden_path):
        return name, False, f"golden missing: {golden_path} (run with --update)"

    with open(golden_path) as f:
        expected = f.read()
    if actual == expected:
        return name, True, ""
    return name, False, _diff_msg(expected, actual)


def _diff_msg(expected, actual):
    import difflib
    diff = difflib.unified_diff(
        expected.splitlines(keepends=True),
        actual.splitlines(keepends=True),
        fromfile="expected", tofile="actual", lineterm="")
    return "screen mismatch\n" + "".join(diff)


def main():
    global UPDATE
    parser = argparse.ArgumentParser()
    parser.add_argument("--update", action="store_true",
                        help="overwrite goldens with the actual dumps")
    args = parser.parse_args()
    UPDATE = args.update

    if not os.path.isdir(CASES_DIR):
        print("FAIL: no cases directory")
        return 2

    if not (os.path.isfile(ITREE) and os.access(ITREE, os.X_OK)):
        print(f"FAIL: itree binary not found or not executable at {ITREE} "
              f"(run 'make build' first)", file=sys.stderr)
        return 2

    cases = sorted(
        os.path.join(CASES_DIR, f)
        for f in os.listdir(CASES_DIR)
        if f.endswith(".py") and not f.startswith("_")
    )
    if not cases:
        print("WARN: no case files found, nothing to do")
        return 0

    n_pass = n_fail = 0
    for case in cases:
        name, ok, msg = run_case(case)
        if ok:
            print(f"PASS {name}" + (f" ({msg})" if msg else ""))
            n_pass += 1
        else:
            print(f"FAIL {name}")
            if msg:
                print("--- detail ---")
                print(msg)
                print("--- end ---")
            n_fail += 1

    print()
    print(f"Dumps: {n_pass} passed, {n_fail} failed")
    return 0 if n_fail == 0 else 1


if __name__ == "__main__":
    sys.exit(main())
