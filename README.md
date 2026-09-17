# posix-shell-c

[![C11](https://img.shields.io/badge/Language-C11-00599C.svg?style=flat-square&logo=c&logoColor=white)](https://en.cppreference.com/w/c/11)
[![Standard](https://img.shields.io/badge/Standard-POSIX.1--2008-blue.svg?style=flat-square)](https://pubs.opengroup.org/onlinepubs/9699919799/)
[![Compiler](https://img.shields.io/badge/Compiler-GCC%20%7C%20Clang-brightgreen.svg?style=flat-square)](https://gcc.gnu.org/)
[![Build Status](https://img.shields.io/badge/Tests-10%2F10%20Passing-success.svg?style=flat-square)](#automated-test-suite)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg?style=flat-square)](LICENSE)

> A lightweight, modular POSIX-compliant UNIX command-line interpreter implemented from scratch in pure C. Features zero external dependencies, robust quote parsing, PATH resolution, internal built-ins, process lifecycle control, and stream redirection.

---

## Architectural Highlights

```
+-------------------------------------------------------------+
|                      REPL Input Loop                        |
|              (getline, signal masking, prompt)              |
+------------------------------+------------------------------+
                               |
                               v
+-------------------------------------------------------------+
|                     Tokenizer & Parser                      |
|         ('...' literal, "..." escape, \, >, >>, 2>, 2>>)    |
+------------------------------+------------------------------+
                               |
                +--------------+--------------+
                |                             |
                v                             v
+-------------------------------+ +---------------------------+
|       Built-in Engine         | |     Executor & Loader     |
|   (cd, pwd, echo, type, exit) | | ($PATH resolution, fork)  |
+---------------+---------------+ +-------------+-------------+
                |                               |
                +---------------+---------------+
                                |
                                v
+-------------------------------------------------------------+
|               File Descriptor Redirection Engine            |
|                  (dup2, open with O_TRUNC / O_APPEND)       |
+-------------------------------------------------------------+
```

### Core Features

1. **State-Machine Tokenizer (`parser.c`)**:
   - **Single Quotes (`'...'`)**: Strictly preserves literal character values without escape interpretation.
   - **Double Quotes (`"..."`)**: Preserves spaces while permitting backslash escapes (`\"`, `\\`, `\$`).
   - **Escape Sequences**: Supports backslash escaping outside quotation marks (`\ ` produces literal space).
2. **Standard Shell Built-ins (`builtins.c`)**:
   - `cd [dir]`: Directory navigation supporting `~` (home directory via `$HOME`) and `-` (previous directory via `$OLDPWD`).
   - `pwd`: Displays current working directory via `getcwd`.
   - `echo [args ...]`: Space-separated argument output.
   - `type [name ...]`: Distinguishes between internal shell built-ins and binaries resolved in `$PATH`.
   - `help`: Internal command documentation.
   - `exit [code]`: Graceful termination with optional exit status codes.
3. **Execution & Path Resolution (`executor.c`)**:
   - Traverses directories in `$PATH` to locate valid regular executables (`S_ISREG` & `X_OK`).
   - Handles absolute paths (`/usr/bin/ls`) and relative paths (`./test.sh`).
   - Manages process lifecycle via `fork()`, `execv()`, and `waitpid()`.
4. **File Descriptor I/O Redirection**:
   - Standard output overwrite: `>` and `1>`
   - Standard output append: `>>` and `1>>`
   - Standard error overwrite: `2>`
   - Standard error append: `2>>`
   - Applies to both internal shell built-ins and external binaries using `dup2()`.

---

## Quick Start

### Prerequisites
* GCC or Clang
* GNU Make
* POSIX-compliant operating system (Linux, macOS, BSD)

### Compilation

```bash
# Clone the repository
git clone https://github.com/AnesuBlessed/posix-shell-c.git
cd posix-shell-c

# Compile with high optimization and strict warnings
make

# Launch the interactive shell
./posix-shell
```

---

## Example Interactive Session

```bash
$ type echo cd ls
echo is a shell builtin
cd is a shell builtin
ls is /usr/bin/ls

$ pwd
/home/anesu/Projects/posix-shell-c

$ echo "Compiling on" 'Linux Kernel' \$(uname -s)
Compiling on Linux Kernel $(uname -s)

$ echo "Logging deployment metrics" > output.log
$ echo "Process completed successfully" >> output.log
$ cat output.log
Logging deployment metrics
Process completed successfully

$ ls /nonexistent_directory 2> error.log
$ cat error.log
ls: cannot access '/nonexistent_directory': No such file or directory

$ cd /tmp
$ pwd
/tmp
$ cd -
/home/anesu/Projects/posix-shell-c
$ exit 0
```

---

## Automated Test Suite

A comprehensive test harness validates built-ins, path resolution, quoting preservation, and redirection mechanics:

```bash
make test
```

### Test Output
```text
=== Running posix-shell Test Suite ===
  ✓ builtin echo simple
  ✓ builtin echo multiple spaces
  ✓ builtin type builtins
  ✓ builtin type external
  ✓ builtin type not found
  ✓ builtin pwd
  ✓ builtin cd and pwd
  ✓ single quotes preserve spaces
  ✓ double quotes simple
  ✓ double quotes escape quote
  ✓ double quotes escape backslash
Testing I/O Redirection...
  ✓ stdout redirection (>)
  ✓ stdout append redirection (>>)
  ✓ stderr redirection (2>)
=== All 10/10 tests passed successfully! ===
```

---

## Project Structure

```text
posix-shell-c/
├── include/
│   ├── builtins.h      # Declarations for internal built-in commands
│   ├── executor.h      # Path resolution and fork/exec prototypes
│   └── parser.h        # Command structs and tokenizer prototypes
├── src/
│   ├── builtins.c      # cd, pwd, echo, type, exit, help implementations
│   ├── executor.c      # Path traversal, child process spawning, redirection
│   ├── main.c          # REPL loop, signal handling, input sanitization
│   └── parser.c        # Quoting state-machine and argument tokenization
├── tests/
│   └── test_shell.sh   # Automated verification script
├── Makefile            # Build configuration with strict flags (-Wall -Werror)
├── .gitignore          # Build artifact exclusions
└── README.md           # Documentation & architectural reference
```

---

## License

Distributed under the **MIT License**. See `LICENSE` for details.