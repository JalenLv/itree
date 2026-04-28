"""Navigate down three rows with j; selection should be on a.txt? No, on c.txt
(root row 0 -> j -> a.txt -> j -> b.txt -> j -> c.txt)."""
FIXTURE = "tests/fixtures/flat"
KEYS = ["j", "j", "j"]
GOLDEN = "02_navigate_jk.dump"
