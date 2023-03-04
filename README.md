# POOP TETRIS

POOP TETRIS stands for OOP and POO (english and portuguese acronyms for Object
Oriented Programming).

A OOP TETRIS game for terminal made with ncurses. All terminal tetris made with
cpp that i found in the internet are did not used OOP and heavily relied on
global variables.

This program is based around 3 classes Piece, Grid and Game. Piece is the
representation of a Tetromino, it contains all the piece data, movement methods
and needed checks. The Grid contains the field game collision check and methods
to mutate grid state e.g. fix a block or clear a line.

The Game class is responsible for orchestrating game logic, executing game loop,
taking user input, drawing on screen (a refactor could be done to composite the
last two responsibilities in different objects).

# INSTALLATION

Install ncurses with your preferred package manager. And run the commands

```
$ mkdir bin
$ clang++ src/main.cc -o ./bin/tetris -lcurses
$ ./bin/tetris
```
