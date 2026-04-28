#!/usr/bin/env python3
"""On-failure diagnostic: re-run dump case 02 (j j j) and print:
  - raw bytes the binary emitted (hex + repr)
  - the rendered pyte screen
This is invoked from CI (build.yml) when 'make test' fails, to help diagnose
why the cell-grid output diverges between platforms."""
import os
import sys
import time

try:
    import pyte
    import pexpect
except ImportError as e:
    print(f"diagnostic skipped: {e}", file=sys.stderr)
    sys.exit(0)

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from _rep import RepExpandingByteStream  # noqa: E402

ROWS, COLS = 24, 80
ITREE = os.environ.get("ITREE", "./itree")

env = os.environ.copy()
env.update({
    "TERM": "xterm-256color",
    "LINES": str(ROWS),
    "COLUMNS": str(COLS),
    "LC_ALL": "C.UTF-8",
})

screen = pyte.Screen(COLS, ROWS)
stream = RepExpandingByteStream(pyte.ByteStream(screen))
captured = bytearray()


def settle(child, idle_ms=400, max_ms=3000):
    deadline = time.time() + max_ms / 1000
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
            captured.extend(buf)
            stream.feed(buf)
            last = time.time()


child = pexpect.spawn(ITREE, ["tests/fixtures/flat"],
                     dimensions=(ROWS, COLS), env=env, encoding=None, timeout=2)
settle(child)
for k in ["j", "j", "j"]:
    child.send(k.encode())
    settle(child)

print("--- raw bytes (hex) ---")
print(bytes(captured).hex())
print()
print("--- raw bytes (repr) ---")
print(repr(bytes(captured)))
print()
print("--- screen.display (rstripped) ---")
for i, row in enumerate(screen.display):
    print(f"{i:02d}: {row.rstrip()!r}")

try:
    child.send(b"q")
    child.expect(pexpect.EOF, timeout=2)
except Exception:
    pass
