# Writing a shell from scratch

This is a simple shell implementation written in C++ but using certain C language elements. It uses UNIX system calls so the current version will not (yet) run on Windows kernels.

## Setup (for Windows)

_For macOS/Linux, jump to step 3_

1. Install WSL with a distro (I have used Ubuntu).

2. Open your distro and run this command:

```bash
    sudo apt update && sudo apt install build-essential

```

3. Navigate to this project directory and run this command:

```bash
    g++ main.cpp -o nash

```

## References

- [Stephen Brennan](https://brennan.io/2015/01/16/write-a-shell-in-c/)

C:\Users\natha\ip\etc\nash_shell\main.cpp
