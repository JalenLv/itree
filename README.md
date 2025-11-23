# itree
Interactive Tree - Interactively View the File Tree and Selectively Collapse/Expand Folders to Build a Desired Text-Based File Tree View just like Tree Does.

## Requirements
- ncursesw (ncurses without wide character support)
- make
- python3
- POSIX-compliant OS

## Build
```bash
./configure
make
make install
```
Note that itree defaults to wide ncurses. To disable wide character support, run `./configure --disable-wide-ncurses` instead.

### MacOS
MacOS comes with ncurses pre-installed, but without ncursesw (wide character support). Use Homebrew to install ncurses with wide character support:
```bash
brew install ncurses
```

## Usage
```bash
itree [directory] [options]
```
