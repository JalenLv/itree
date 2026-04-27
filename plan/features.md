`tree` has the following flags:

```bash
tree  [-acdfghilnpqrstuvxACDFJQNSUX]  [-L  level [-R]] [-H [-]baseHREF] [-T title] [-o filename]
[-P pattern] [-I pattern] [--gitignore] [--gitfile[=]file]  [--matchdirs]  [--metafirst]  [--ig‐
nore-case] [--nolinks] [--hintro[=]file] [--houtro[=]file] [--inodes] [--device] [--sort[=]name]
[--dirsfirst]   [--filesfirst]   [--filelimit[=]number]   [--si]   [--du]  [--prune]  [--charset[=]X]
[--timefmt[=]format]  [--fromfile]  [--fromtabfile]  [--fflinks]  [--info]   [--infofile[=]file]
[--noreport]  [--hyperlink]  [--scheme[=]schema] [--authority[=]hostname] [--opt-toggle] [--ver‐
sion] [--help] [--] [directory ...]
```
---

- [X] Add sorting: stable alphabetical order.
- [ ] Add flags like -a/--all, --max-depth N, --dirs-only, and --ignore PATTERN.
- [X] Add --no-tui flag.
- [ ] Add search/filter in the TUI with /pattern, then jump between matches with n / N.
- [ ] Add .gitignore-aware filtering. This would make the tool much more useful in real repos.
- [ ] Add TUI help/status bar showing controls: j/k, h/l, g/G, Ctrl-D/U, q.
- [ ] Add expand/collapse all commands: likely zM / zR, or simple H / L.
- [ ] Add output styles: Unicode tree, ASCII tree, JSON, and maybe Markdown code block.
