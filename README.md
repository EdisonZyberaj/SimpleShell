# Simple Linux Shell Implementation

A lightweight shell implementation in C that supports command execution, built-in commands, pipes, and I/O redirection.

## Features

- **Command Execution**: Run system commands
- **Built-in Commands**: 
  - `cd`: Change directory
  - `pwd`: Print working directory
  - `echo`: Display text
  - `help`: Show available commands
  - `history`: Show command history
  - `clear`: Clear the screen
  - `exit`: Exit the shell
- **I/O Redirection**:
  - `>`: Redirect output to file
  - `<`: Redirect input from file
- **Pipes**: Connect commands with `|`
- **Background Processing**: Run commands in background with `&`
- **Command History**: Store and recall previous commands
- **Colorful Prompt**: Shows current directory with color

## Files

- `main.c`: Main shell implementation and command processing
- `shell.h`: Header file with definitions and function declarations
- `utils.c`: Implementation of built-in commands and utilities

## Building and Running

### Prerequisites

- GCC compiler
- Linux environment

### Compilation

```bash
gcc main.c utils.c -o shell
```

### Running

```bash
./shell
```

## Usage Examples

### Basic Commands

```
~/projects$> ls
```

### I/O Redirection

```
~/projects$> ls > file_list.txt
~/projects$> wc < file_list.txt
```

### Pipes

```
~/projects$> ls | grep .c
```

### Background Processing

```
~/projects$> sleep 10 &
```

## Code Structure

- **Command Parsing**: `parse_command()` splits user input into arguments
- **Redirection Handling**: `check_redirections()` processes I/O redirection
- **Command Execution**: `execute_command()` runs commands using execvp
- **Pipe Handling**: `execute_pipe()` creates processes connected by a pipe
- **Built-in Commands**: Implemented in `utils.c` with command history

## Implementation Details

The shell operates through a simple loop:
1. Display prompt
2. Read user input
3. Parse input into command and arguments
4. Check for built-in commands
5. Handle redirections and pipes
6. Fork and execute commands
7. Wait for command completion (unless in background)
8. Repeat

---

Made by Edison Zyberaj
