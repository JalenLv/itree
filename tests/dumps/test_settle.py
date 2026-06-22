#!/usr/bin/env python3
"""Unit tests for the TUI dump runner's settle helper."""

import os
import sys
import unittest
from unittest.mock import patch

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import runner


TIMEOUT = object()
EOF = object()


class FakeClock:
    def __init__(self):
        self.now = 0.0

    def __call__(self):
        return self.now

    def advance(self, milliseconds):
        self.now += milliseconds / 1000.0


class ScriptedChild:
    def __init__(self, clock, events=(), default=(50, TIMEOUT)):
        self.clock = clock
        self.events = list(events)
        self.default = default

    def read_nonblocking(self, size, timeout):
        del size, timeout
        delay, result = self.events.pop(0) if self.events else self.default
        self.clock.advance(delay)

        if result is TIMEOUT:
            raise runner.pexpect.TIMEOUT("scripted timeout")
        if result is EOF:
            raise runner.pexpect.EOF("scripted EOF")
        return result


class CapturingStream:
    def __init__(self):
        self.buf = bytearray()

    def feed(self, data):
        self.buf.extend(data)


class SettleTests(unittest.TestCase):
    def settle(self, child, clock, stream, **kwargs):
        with patch.object(runner.time, "monotonic", clock), \
                patch.object(runner.time, "time", clock):
            runner.settle(child, None, stream, **kwargs)

    def test_pre_output_silence_does_not_count_as_idle(self):
        clock = FakeClock()
        stream = CapturingStream()
        child = ScriptedChild(clock, events=[
            (75, TIMEOUT), (75, TIMEOUT), (75, TIMEOUT), (0, b"frame"),
        ])

        self.settle(child, clock, stream, idle_ms=200, max_ms=1000)

        self.assertEqual(bytes(stream.buf), b"frame")

    def test_no_output_reports_timeout(self):
        clock = FakeClock()
        with self.assertRaisesRegex(
                TimeoutError, "no terminal output within 250 ms"):
            self.settle(
                ScriptedChild(clock), clock, CapturingStream(),
                idle_ms=200, max_ms=250)

    def test_continuous_output_reports_timeout(self):
        clock = FakeClock()
        child = ScriptedChild(clock, default=(50, b"x"))
        with self.assertRaisesRegex(
                TimeoutError, "output did not settle within 250 ms"):
            self.settle(
                child, clock, CapturingStream(),
                idle_ms=200, max_ms=250)

    def test_eof_before_output_reports_early_exit(self):
        clock = FakeClock()
        child = ScriptedChild(clock, events=[(0, EOF)])
        with self.assertRaisesRegex(
                RuntimeError, "exited before producing terminal output"):
            self.settle(
                child, clock, CapturingStream(),
                idle_ms=200, max_ms=1000)

    def test_eof_after_output_succeeds(self):
        clock = FakeClock()
        stream = CapturingStream()
        child = ScriptedChild(clock, events=[
            (0, b"complete frame"), (10, EOF),
        ])

        self.settle(child, clock, stream, idle_ms=200, max_ms=1000)

        self.assertEqual(bytes(stream.buf), b"complete frame")


if __name__ == "__main__":
    unittest.main()
