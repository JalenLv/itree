"""Pyte ByteStream wrapper that expands CSI REP (`ESC [ Pn b`) before feeding.

ECMA-48 REP repeats the preceding graphic character `Pn` times. ncurses uses
it as a length optimization for runs of identical characters when the active
terminfo's `rep` capability is set. pyte 0.8.2 doesn't implement REP — its
CSI dispatch routes the unknown final byte 'b' to a no-op `Screen.debug()` —
so on hosts where ncurses optimizes (e.g., macos-14 with Homebrew ncursesw
6.6 + xterm-256color) the repeated cells vanish from `screen.display`.

This wrapper sits between `pexpect.spawn`'s output and `pyte.ByteStream`,
walking each chunk byte-by-byte, tracking the most recent printable byte
forwarded, and replacing any `\\x1b[<digits>b` with that many literal copies
before passing the rewritten stream onward. Partial CSI sequences split
across chunk boundaries are held in `_partial` until the next feed.
"""
import re

# A CSI sequence: ESC '[' <param-bytes 0x30-0x3F>* <intermediate-bytes 0x20-0x2F>* <final 0x40-0x7E>
_CSI_RE = re.compile(rb"\x1b\[([\x30-\x3f]*)[\x20-\x2f]*([\x40-\x7e])")


class RepExpandingByteStream:
    def __init__(self, inner):
        """`inner` is a pyte.ByteStream (or anything with a `feed(bytes)` method)."""
        self._inner = inner
        self._last_graphic = b""   # most recent printable byte forwarded
        self._partial = b""        # trailing partial CSI carried to next feed

    def feed(self, data):
        if not isinstance(data, (bytes, bytearray)):
            raise TypeError("feed() expects bytes")
        buf = self._partial + bytes(data)
        self._partial = b""
        out = bytearray()
        i = 0
        n = len(buf)
        while i < n:
            b = buf[i]
            if b != 0x1b:
                # Plain byte. If printable ASCII, remember it for any future REP.
                if 0x20 <= b <= 0x7e:
                    self._last_graphic = bytes((b,))
                out.append(b)
                i += 1
                continue
            # ESC. Need at least one more byte to know what follows.
            if i + 1 >= n:
                self._partial = buf[i:]
                break
            if buf[i + 1] != 0x5b:  # not '[', so not CSI; pass ESC through
                out.append(b)
                i += 1
                continue
            # We have ESC '['. Try to match a complete CSI starting at i.
            m = _CSI_RE.match(buf, i)
            if not m:
                # Incomplete CSI; hold from here for the next feed.
                self._partial = buf[i:]
                break
            params = m.group(1)
            final = m.group(2)
            if final == b"b":
                # REP: repeat _last_graphic Pn times. Default Pn = 1.
                try:
                    count = int(params) if params else 1
                except ValueError:
                    # Malformed REP params; pass the original bytes through.
                    out.extend(buf[i:m.end()])
                    i = m.end()
                    continue
                if self._last_graphic and count > 0:
                    out.extend(self._last_graphic * count)
                # _last_graphic stays the same — repeats are of the same char.
            else:
                # Non-REP CSI (SGR, cursor moves, erase, etc.) — forward unchanged.
                out.extend(buf[i:m.end()])
            i = m.end()
        if out:
            self._inner.feed(bytes(out))
