# itree
Interactive Tree - Interactively View the File Tree and Selectively Collapse/Expand Folders to Build a Desired Text-Based File Tree View just like Tree Does.

## Requirements
- ncurses (ncursesw if enabled wide character support)
- make
- POSIX-compliant OS

## Build
```bash
./configure
make
make install
```
Note that itree defaults to "narrow" ncurses. To enable wide character support, run `./configure --enable-wide-ncurses` instead.

### MacOS
MacOS comes with ncurses pre-installed, but without ncursesw (wide character support). Use Homebrew to install ncurses with wide character support:
```bash
brew install ncurses
```
The `configure` script should then automatically detect the Homebrew-installed ncurses. It's worth noting that Homebrew symlinks ncurses to ncursesw, so `--enable-wide-ncurses` may not be necessary.

## Usage
```bash
itree [directory] [options]
```
