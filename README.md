# LinuxGameShelf - A Terminal-Based Game Launcher

## Project Overview
LinuxGameShelf (shelf-steam) is a terminal-based game launcher that provides a simplified version of Steam for the Linux shell. It allows users to search, browse, and run games through a command-line interface. This project was developed as part of a systems programming assignment to demonstrate understanding of process management, file I/O, and shell functionality in Linux.

## Features
- Interactive command prompt (`shelf-steam>`)
- Built-in commands:
  - `exit`: Exits the program
  - `ls`: Lists all files in the game repository with descriptions
  - `path`: Changes the current game repository path
- Ability to run executable games from the specified repository
- Standard input redirection using the `<` operator
- Robust error handling

## Installation

### Prerequisites
- Linux operating system
- GCC compiler
- Make

### Building from Source
1. Clone the repository:
```bash
git clone https://github.com/yourusername/LinuxGameShelf.git
cd LinuxGameShelf
```

2. Compile the program:
```bash
make
```

This will create an executable named `shelf-steam` in the current directory.

## Usage

### Running ShelfSteam
```bash
./shelf-steam /path/to/games/
```

Where `/path/to/games/` is the path to your game repository. This directory should contain executable games that can be run through ShelfSteam.

### Built-in Commands

#### exit
Exits the ShelfSteam program.
```
shelf-steam> exit
```

#### ls
Lists all files in the current game repository with their descriptions.
```
shelf-steam> ls
```

#### path
Changes the current game repository path.
```
shelf-steam> path /new/path/to/games/
```

### Running Games
To run a game from the repository, simply type its name at the prompt.
```
shelf-steam> gamename [arguments]
```

### Input Redirection
You can redirect input to games using the `<` operator.
```
shelf-steam> gamename [arguments] < input_file.txt
```

## Error Handling
ShelfSteam handles various error scenarios, including:
- Invalid command-line arguments
- Attempting to access non-existent files or directories
- Executing non-executable files
- Invalid syntax for built-in commands
- Failed process creation

## Code Structure
- `shelf-steam.c`: Main source file containing the shell implementation
- `Makefile`: Build file for compiling the project


## Author
[Rishil Shah]
