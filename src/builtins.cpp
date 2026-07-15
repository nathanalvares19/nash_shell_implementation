#include "builtins.h"
#include "constants.h"
#include "globals.h"

#include <iostream>
#include <iomanip>
#include <unistd.h>
#include <cstring>

// variable and data structure declarations
extern char *wk_dir;
extern int history_last_insert_idx;
extern int history_count;
extern char *nash_history_lines[];

// built-in function names
const char *nash_builtins_str[] = {
    "cd",
    "help",
    "exit",
    "loc",
    "slate",
    "past",
    "jobs"};

// built-in function descriptions
const char *nash_builtins_desc[] = {
    "cd: Changes the current directory",
    "help: Information on available commands",
    "exit: Exits the shell",
    "loc: Prints the current directory",
    "slate: Clears the terminal screen",
    "past: Prints the command history for the current shell session",
    "jobs: Prints all the running background jobs"};

// array of built-in functions
int (*nash_builtins_func[])(char **) = {
    nash_cd,
    nash_help,
    nash_exit,
    nash_pwd,
    nash_clear,
    nash_history,
    nash_jobs};

// built-in function count
int nash_builtins_nums()
{
    return sizeof(nash_builtins_str) / sizeof(const char *);
}

// change directory function
int nash_cd(char **args)
{
    if (args[1] == nullptr)
    {
        // std::cerr << "nash: expected argument for \"cd\"\n";
        if (chdir("/") != 0)
        {
            perror("nash");
        }
    }
    else
    {
        if (chdir(args[1]) != 0)
        {
            perror("nash");
        }
    }

    // save working dir in variable
    if (getcwd(wk_dir, WDIR_BUFSIZE) == nullptr)
    {
        perror("getcwd");
    }

    return 1;
}

// help function
int nash_help(char **args)
{
    std::cout << "\n------------------------------------------------\n";
    std::cout << R"(░███    ░██    ░███      ░██████   ░██     ░██ 
░████   ░██   ░██░██    ░██   ░██  ░██     ░██ 
░██░██  ░██  ░██  ░██  ░██         ░██     ░██ 
░██ ░██ ░██ ░█████████  ░████████  ░██████████ 
░██  ░██░██ ░██    ░██         ░██ ░██     ░██ 
░██   ░████ ░██    ░██  ░██   ░██  ░██     ░██ 
░██    ░███ ░██    ░██   ░██████   ░██     ░██ 
                                               )";
    std::cout << "\n";
    std::cout << "Type program names and arguments, and hit enter.\n\n";
    std::cout << "The following are built-in:\n";

    for (int i = 0; i < nash_builtins_nums(); i++)
    {
        std::cout << (i + 1) << ". " << nash_builtins_desc[i] << "\n";
    }

    std::cout << "\nMore functionality to be added soon.\n";
    std::cout << "------------------------------------------------\n\n";

    return 1;
}

// exit function
int nash_exit(char **args)
{
    std::cout << "\nExiting Shell...\n\n";
    return 0;
}

// print working directory function
int nash_pwd(char **args)
{
    if (getcwd(wk_dir, WDIR_BUFSIZE) == nullptr)
    {
        perror("nash");
    }
    std::cout << wk_dir << "\n";
    return 1;
}

// clear terminal screen function
int nash_clear(char **args)
{
    std::cout << "\033[H\033[2J";
    return 1;
}

// print command history function
int nash_history(char **args)
{
    history.print();
    return 1;
}

// print jobs function
int nash_jobs(char **args)
{
    shell.print_jobs();
    return 1;
}