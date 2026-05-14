#!/usr/bin/env bash
# Black-box integration tests for itree. Runs the built binary against
# fixture directories and diffs stdout against the matching golden files.
# Run from the project root: ./tests/integration/run.sh
#
# Each test prints "PASS <name>" or "FAIL <name>" and a non-zero overall
# exit code if any case fails.

set -u

ITREE=${ITREE:-./itree}
FIXTURES=tests/fixtures
GOLDEN=tests/golden

if [[ ! -x "$ITREE" ]]; then
    echo "FAIL: itree binary not found at $ITREE (run 'make build' first)" >&2
    exit 2
fi

pass=0
fail=0

# Run a command that should succeed (exit 0) and assert its stdout matches a
# golden file. Writes stdout to a tempfile so trailing newlines are preserved
# (bash's $(...) strips them).
expect_match() {
    local name=$1 golden=$2; shift 2
    local tmp
    tmp=$(mktemp -t itree-int-XXXXXX)
    if ! "$@" >"$tmp" 2>/dev/null; then
        echo "FAIL $name (binary returned non-zero)"
        fail=$((fail + 1))
        rm -f "$tmp"
        return
    fi
    if diff -u "$tmp" "$golden" >"$tmp.diff" 2>&1; then
        echo "PASS $name"
        pass=$((pass + 1))
    else
        echo "FAIL $name"
        echo "--- diff ---"
        cat "$tmp.diff"
        echo "--- end ---"
        fail=$((fail + 1))
    fi
    rm -f "$tmp" "$tmp.diff"
}

# Run a command that should fail (non-zero exit).
expect_fail() {
    local name=$1; shift
    if "$@" >/dev/null 2>&1; then
        echo "FAIL $name (expected non-zero exit but got 0)"
        fail=$((fail + 1))
    else
        echo "PASS $name"
        pass=$((pass + 1))
    fi
}

# ---------------------------------------------------------------------------
# Help & flag discovery
# ---------------------------------------------------------------------------

help_out=$("$ITREE" --help 2>/dev/null)
help_rc=$?
if [[ $help_rc -ne 0 ]]; then
    echo "FAIL help_exits_zero"
    fail=$((fail + 1))
else
    echo "PASS help_exits_zero"
    pass=$((pass + 1))
fi
for flag in "--no-tui" "--show-hidden" "--output" "--help" "--version"; do
    if grep -q -- "$flag" <<<"$help_out"; then
        echo "PASS help_mentions_${flag#--}"
        pass=$((pass + 1))
    else
        echo "FAIL help_mentions_${flag#--}"
        fail=$((fail + 1))
    fi
done

# Same flags via -h.
if "$ITREE" -h >/dev/null 2>&1; then
    echo "PASS short_h_exits_zero"
    pass=$((pass + 1))
else
    echo "FAIL short_h_exits_zero"
    fail=$((fail + 1))
fi

# ---------------------------------------------------------------------------
# Version flag
# ---------------------------------------------------------------------------

expect_match version_long  "$GOLDEN/version.txt" "$ITREE" --version
expect_match version_short "$GOLDEN/version.txt" "$ITREE" -v

# ---------------------------------------------------------------------------
# Golden-file matches
# ---------------------------------------------------------------------------

expect_match flat               "$GOLDEN/flat.txt"               "$ITREE" --no-tui "$FIXTURES/flat"
expect_match nested             "$GOLDEN/nested.txt"             "$ITREE" --no-tui "$FIXTURES/nested"
expect_match mixed_case         "$GOLDEN/mixed_case.txt"         "$ITREE" --no-tui "$FIXTURES/mixed_case"
expect_match with_hidden_no_a   "$GOLDEN/with_hidden.no_a.txt"   "$ITREE" --no-tui "$FIXTURES/with_hidden"
expect_match with_hidden_with_a "$GOLDEN/with_hidden.with_a.txt" "$ITREE" --no-tui -a "$FIXTURES/with_hidden"
expect_match with_link          "$GOLDEN/with_link.txt"          "$ITREE" --no-tui "$FIXTURES/with_link"

# -o writes to file
tmpout=$(mktemp -t itree-int-XXXXXX)
if "$ITREE" --no-tui -o "$tmpout" "$FIXTURES/flat" >/dev/null 2>&1 \
   && diff -u "$tmpout" "$GOLDEN/flat.txt" >/tmp/itree-int-$$-o.diff 2>&1; then
    echo "PASS output_flag_writes_to_file"
    pass=$((pass + 1))
else
    echo "FAIL output_flag_writes_to_file"
    [[ -f /tmp/itree-int-$$-o.diff ]] && cat /tmp/itree-int-$$-o.diff
    fail=$((fail + 1))
fi
rm -f "$tmpout" /tmp/itree-int-$$-o.diff

# Empty directory (created at runtime so we don't need git to track an empty dir)
empty_dir=$(mktemp -d -t itree-int-empty-XXXXXX)
basename_dir=$(basename "$empty_dir")
out=$("$ITREE" --no-tui "$empty_dir" 2>/dev/null)
if [[ "$out" == "$basename_dir/" ]]; then
    echo "PASS empty_directory"
    pass=$((pass + 1))
else
    echo "FAIL empty_directory (got '$out', expected '$basename_dir/')"
    fail=$((fail + 1))
fi
rmdir "$empty_dir"

# ---------------------------------------------------------------------------
# Error paths
# ---------------------------------------------------------------------------

expect_fail nonexistent_path_errors        "$ITREE" --no-tui /this/path/does/not/exist
expect_fail two_paths_error                "$ITREE" --no-tui "$FIXTURES/flat" "$FIXTURES/nested"
expect_fail unknown_option_error           "$ITREE" --this-flag-is-not-real

# ---------------------------------------------------------------------------
# Summary
# ---------------------------------------------------------------------------
echo
echo "Integration: $pass passed, $fail failed"
if [[ $fail -gt 0 ]]; then
    exit 1
fi
exit 0
