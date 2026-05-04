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
- [X] Add --no-tui flag.
- [X] Add -a/--hidden flag.
- [X] Add left/right arrow to move left/right in the TUI in case of long file names.
- [X] Add mouse support: click to select, double-click to expand/collapse, scroll to move up/down.
- [ ] Add search/filter in the TUI with /pattern, then jump between matches with n / N.
- [ ] Add --dirsfirst and --filesfirst flags.
- [ ] Add d to hide entries, u to undo hiding, U to unhide all, and r to undo the undos.
- [ ] Add --gitignore flag.
- [ ] Add toggleable show hidden option in the TUI.
- [ ] Add a floating help panel toggled with ?, showing all keybindings and their descriptions.
- [ ] Add expand/collapse all commands: likely zM / zR, or simple H / L.
- [ ] Add output styles: Unicode tree, ASCII tree, JSON, and maybe Markdown code block.
