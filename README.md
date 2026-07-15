# Writing a shell from scratch

A simple Unix-like shell written in C++.

## Prerequisites

### Windows (WSL)

1. Install WSL with a Linux distribution (Ubuntu recommended).

2. Open your WSL terminal and install the build tools:

```bash
sudo apt update
sudo apt install build-essential
```

### macOS

Install Xcode Command Line Tools:

```bash
xcode-select --install
```

### Linux

Install a C++ compiler and Make:

```bash
sudo apt update
sudo apt install build-essential
```

---

## Build

From the project root directory:

```bash
make
```

This will generate the executable:

```text
nash
```

---

## Run

```bash
./nash
```

Or:

```bash
make run
```

---

## Clean Build Files

```bash
make clean
```

---

## Rebuild

```bash
make rebuild
```

## Features

- Command execution
- Built-in commands (`cd`, `help`, `exit`, etc.)
- Command history
- Multiple pipes support
- Custom prompt
- Signal handling
- Non-canonical terminal input
- Background jobs
- Job control

## References

- [Stephen Brennan](https://brennan.io/2015/01/16/write-a-shell-in-c/)
- [POSIX man-pages](https://man7.org/linux/man-pages/dir_all_by_section.html)
- [ANSI Escape Codes](https://gist.github.com/fnky/458719343aabd01cfb17a3a4f7296797)
- [POSIX Signals man-page](https://www.cs.kent.edu/~ruttan/sysprog/lectures/signal.7.html)
