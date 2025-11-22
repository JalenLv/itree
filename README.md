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

## Usage
```bash
itree [directory] [options]
```
