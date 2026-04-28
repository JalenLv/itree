#!/usr/bin/env python3
"""Unit tests for tests/dumps/_rep.py — the CSI REP expander.

Standalone: `python tests/dumps/test_rep.py`. Returns non-zero on failure.
Doesn't depend on pyte; uses a tiny CapturingStream stand-in that just
records the bytes its feed() is called with.
"""
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from _rep import RepExpandingByteStream


class CapturingStream:
    def __init__(self):
        self.buf = bytearray()

    def feed(self, data):
        self.buf.extend(data)


def make():
    cap = CapturingStream()
    return RepExpandingByteStream(cap), cap


def test_simple_rep():
    s, cap = make()
    s.feed(b"a\x1b[3b")
    assert bytes(cap.buf) == b"aaaa", bytes(cap.buf)


def test_rep_no_param_means_one():
    s, cap = make()
    s.feed(b"X\x1b[b")
    assert bytes(cap.buf) == b"XX", bytes(cap.buf)


def test_rep_across_two_feeds():
    s, cap = make()
    s.feed(b" \x1b[7b")
    s.feed(b"c")
    assert bytes(cap.buf) == b"        c", bytes(cap.buf)


def test_partial_csi_split_in_params():
    s, cap = make()
    s.feed(b" \x1b[7")
    # The partial CSI must be held back; only the leading space is forwarded.
    assert bytes(cap.buf) == b" ", bytes(cap.buf)
    s.feed(b"b")
    assert bytes(cap.buf) == b"        ", bytes(cap.buf)


def test_partial_csi_just_esc():
    s, cap = make()
    s.feed(b"a\x1b")
    assert bytes(cap.buf) == b"a", bytes(cap.buf)
    s.feed(b"[3b")
    # 'a' (literal, already forwarded) + 3 reps = 4 a's total in cap.
    assert bytes(cap.buf) == b"aaaa", bytes(cap.buf)


def test_partial_csi_just_esc_bracket():
    s, cap = make()
    s.feed(b"z\x1b[")
    assert bytes(cap.buf) == b"z", bytes(cap.buf)
    s.feed(b"4b")
    assert bytes(cap.buf) == b"zzzzz", bytes(cap.buf)


def test_non_rep_csi_passes_through_unchanged():
    s, cap = make()
    payload = b"\x1b[0;7m->"
    s.feed(payload)
    assert bytes(cap.buf) == payload, bytes(cap.buf)


def test_last_graphic_tracks_only_printable():
    """Control bytes inside a CSI must NOT become _last_graphic."""
    s, cap = make()
    # SGR sets attribute, then we write 'Z', then REP 3.
    # The 'last graphic' should be 'Z', not the digits inside the SGR.
    s.feed(b"\x1b[0;7mZ\x1b[3b")
    assert bytes(cap.buf) == b"\x1b[0;7mZZZZ", bytes(cap.buf)


def test_multiple_reps_with_changing_char():
    s, cap = make()
    s.feed(b"a\x1b[2bb\x1b[3b")
    # 'a' + 2 'a' (REP 2) = "aaa", then 'b' becomes new last_graphic, then 3 'b' = "bbbb"
    # Total: aaabbbb
    assert bytes(cap.buf) == b"aaabbbb", bytes(cap.buf)


def test_empty_feed_is_noop():
    s, cap = make()
    s.feed(b"")
    assert bytes(cap.buf) == b"", bytes(cap.buf)


def test_real_macos_byte_run():
    """The exact bytes captured by the CI diagnostic for the failing dump."""
    s, cap = make()
    # Three consecutive feeds simulating ncurses' selected-row repaint.
    s.feed(b"\x1b(B\x1b[0;7m-> \x1b[7bc.txt")
    # Expected expansion: ESC(B + ESC[0;7m + "->" + " " + 7 reps of " " + "c.txt"
    expected = b"\x1b(B\x1b[0;7m->        c.txt"
    assert bytes(cap.buf) == expected, bytes(cap.buf)


def main():
    tests = [
        test_simple_rep,
        test_rep_no_param_means_one,
        test_rep_across_two_feeds,
        test_partial_csi_split_in_params,
        test_partial_csi_just_esc,
        test_partial_csi_just_esc_bracket,
        test_non_rep_csi_passes_through_unchanged,
        test_last_graphic_tracks_only_printable,
        test_multiple_reps_with_changing_char,
        test_empty_feed_is_noop,
        test_real_macos_byte_run,
    ]
    failed = 0
    for t in tests:
        try:
            t()
            print(f"PASS {t.__name__}")
        except AssertionError as e:
            failed += 1
            print(f"FAIL {t.__name__}: got {e!r}")
        except Exception as e:
            failed += 1
            print(f"FAIL {t.__name__}: {type(e).__name__}: {e}")
    print()
    print(f"REP wrapper: {len(tests) - failed} passed, {failed} failed")
    return 0 if failed == 0 else 1


if __name__ == "__main__":
    sys.exit(main())
