"""Open with_hidden with -a; dotfiles should appear in the rendered TUI."""
FIXTURE = "tests/fixtures/with_hidden"
EXTRA_ARGV = ["-a"]
KEYS = []
GOLDEN = "05_show_hidden_with_a_flag.dump"
