/*
 * ============================================================================
 * File        : main.cpp
 * Project     : Nash Shell
 * Author      : Nathan Alvares
 * Description : A Unix-like shell built from scratch in C++ using POSIX system
 *               calls. The project is intended for learning.
 * ============================================================================
 */

#include "builtins.h"

#include <iostream>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <sys/wait.h>
#include <string>
#include <iomanip>
#include <termios.h>

// signal handling
struct sigaction sa;

// canonical mode
struct termios original_termios;

// non-canonical mode
struct termios raw = original_termios;

/*
    MACROS
*/

// COLOURS
#define BLACK "\033[30m"
#define RED "\033[31m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define BLUE "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN "\033[36m"
#define WHITE "\033[37m"

// RESET COLOUR
#define RESET "\033[0m"

// read line function
#define NASH_RL_BUFSIZE 1024

// split line function
#define NASH_TOK_BUFSIZE 64
#define NASH_TOK_DELIM " \t\r\n\a"

// version number
#define NASH_VERSION "1.0.0"

// username size
#define USERNAME_BUFSIZE 32

// working directory size
#define WDIR_BUFSIZE 128

// history size
#define HIST_BUFSIZE 128

/*
    TERMINAL SETTINGS
*/

char *name;

void set_ncanonical_mode()
{
    raw.c_lflag &= ~ICANON; // disable canonical mode
    raw.c_lflag &= ~ECHO;   // disable echo
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

// reset canonical mode
void restore_terminal()
{
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &original_termios);
}

// SIGINT handler
void sigint_handler(int sig)
{
    write(STDOUT_FILENO, "^C", 2);
    return;
}

// set signal handler
void install_signal_handlers()
{
    sa.sa_handler = sigint_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, nullptr);
}

/*
    FUNCTION DECLARATIONS
*/

// execution
void nash_loop(void);
int nash_execute(char **args);
int nash_launch(char **args);

// processing
char *nash_read_line(void);
char **nash_split_line(char *line);

// built-in functions
int nash_cd(char **args);
int nash_help(char **args);
int nash_exit(char **args);
int nash_pwd(char **args);
int nash_clear(char **args);
int nash_history(char **args);
int nash_builtins_nums();

// helper functions
void nash_add_history(char *line);
char *process_name();
int process_pipe(char **args);

// helper variables
int history_last_insert_idx = -1;
int history_count = 0;

/*
    FUNCTION DEFINITIONS
*/

// working directory variable
char *wk_dir = (char *)std::malloc(sizeof(char) * WDIR_BUFSIZE);

// built-in functions
const char *nash_builtins_str[] = {
    "cd",
    "help",
    "exit",
    "loc",
    "slate",
    "past"};

// built-in function descriptions
const char *nash_builtins_desc[] = {
    "cd: Changes the current directory",
    "help: Information on available commands",
    "exit: Exits the shell",
    "loc: Prints the current directory",
    "slate: Clears the terminal screen",
    "past: Prints the command history for the current shell session"};

// history cmd lines array
char *nash_history_lines[HIST_BUFSIZE];

int (*nash_builtins_func[])(char **) = {
    nash_cd,
    nash_help,
    nash_exit,
    nash_pwd,
    nash_clear,
    nash_history};

int nash_builtins_nums()
{
    return sizeof(nash_builtins_str) / sizeof(const char *);
}

// CHANGE DIRECTORY FUNCTION
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

// HELP FUNCTION
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

// EXIT FUNCTION
int nash_exit(char **args)
{
    std::cout << "\nExiting Shell...\n\n";
    return 0;
}

// PWD FUNCTION
int nash_pwd(char **args)
{
    if (getcwd(wk_dir, WDIR_BUFSIZE) == nullptr)
    {
        perror("nash");
    }
    std::cout << wk_dir << "\n";
    return 1;
}

// CLEAR TERMINAL FUNCTION
int nash_clear(char **args)
{
    std::cout << "\033[H\033[2J";
    return 1;
}

// ADD TO HISTORY FUNCTION
void nash_add_history(char *line)
{
    history_last_insert_idx = (history_last_insert_idx + 1) % HIST_BUFSIZE;
    history_count += 1;

    if (history_count > 128)
    {
        free(nash_history_lines[history_last_insert_idx]);
    }

    char *cmd = strdup(line);
    if (cmd == nullptr)
    {
        perror("strdup");
        return;
    }
    nash_history_lines[history_last_insert_idx] = cmd;
}

// PRINT HISTORY FUNCTION
int nash_history(char **args)
{
    if (history_count > 128)
    {
        int idx = (history_last_insert_idx + 1) % HIST_BUFSIZE;
        int num = 1;
        while (idx != history_last_insert_idx)
        {
            std::cout << "  " << std::setw(3) << num << "  " << nash_history_lines[idx] << "\n";
            idx = (idx + 1) % HIST_BUFSIZE;
            num += 1;
        }
        std::cout << "  " << std::setw(3) << num << "  " << nash_history_lines[idx] << "\n";
    }
    else
    {
        for (int i = 0; i <= history_last_insert_idx; i++)
        {
            std::cout << "  " << std::setw(3) << (i + 1) << "  " << nash_history_lines[i] << "\n";
        }
    }

    return 1;
}

// username processing
char *process_name()
{
    char *name = (char *)std::malloc(sizeof(char) * USERNAME_BUFSIZE);
    int position = 0;
    if (!name)
    {
        std::cerr << "nash: allocation error\n";
        std::exit(EXIT_FAILURE);
    }

    while (true)
    {
        int c = getchar();
        if (c == '\n')
        {
            std::cout << '\n';
            std::cout.flush();
            break;
        }

        if (c == 127 || c == '\b')
        {
            if (position != 0)
            {
                position--;
                std::cout << "\b \b";
                std::cout.flush();
            }

            continue;
        }

        name[position] = (char)c;
        position++;

        std::cout << (char)c;
        std::cout.flush();
    }

    return name;
}

// pipe processing
int process_pipe(char **args)
{
    // find pipe
    int pipe_idx = -1;
    for (int i = 0; args[i] != nullptr; i++)
    {
        if (strcmp(args[i], "|") == 0)
        {
            pipe_idx = i;
            break;
        }
    }

    // pipe existence check
    if (pipe_idx == -1)
    {
        return 0;
    }
    else if (pipe_idx == 0 || args[pipe_idx + 1] == nullptr)
    {
        std::cerr << "nash: invalid pipe usage\n";
        return 0;
    }

    // split args around pipe
    args[pipe_idx] = nullptr;
    char **left = args;
    char **right = &args[pipe_idx + 1];

    // create a pipe
    int fd[2];
    if (pipe(fd) == -1)
    {
        perror("pipe");
        return 1;
    }

    // left child
    pid_t pid1 = fork();
    if (pid1 == 0) // child process
    {
        if (dup2(fd[1], STDOUT_FILENO) == -1)
        {
            perror("dup2");
            return 1;
        }

        close(fd[0]);
        close(fd[1]);

        if (execvp(left[0], left) == -1)
        {
            perror("nash");
        }
        exit(EXIT_FAILURE);
    }
    else if (pid1 < 0)
    {
        perror("nash");
        return 1;
    }

    // right child
    pid_t pid2 = fork();
    if (pid2 == 0) // child process
    {
        if (dup2(fd[0], STDIN_FILENO) == -1)
        {
            perror("dup2");
            return 1;
        }

        close(fd[0]);
        close(fd[1]);

        if (execvp(right[0], right) == -1)
        {
            perror("nash");
        }
        exit(EXIT_FAILURE);
    }
    else if (pid2 < 0)
    {
        perror("nash");
        return 1;
    }

    // close parent fds
    close(fd[0]);
    close(fd[1]);

    waitpid(pid1, nullptr, 0);
    waitpid(pid2, nullptr, 0);

    return 1;
}

// main shell loop
void nash_loop(void)
{
    char *line;
    char **args;
    int status;

    // username implementation
    std::cout << "Enter username (maximum 32 chars): ";
    // global name variable
    name = process_name();

    if (getcwd(wk_dir, WDIR_BUFSIZE) == nullptr)
    {
        perror("getcwd");
    }

    do
    {
        std::cout << YELLOW << name << RESET << " @ " << GREEN << wk_dir << RESET << " > ";
        line = nash_read_line();
        nash_add_history(line);
        args = nash_split_line(line);
        status = nash_execute(args);

        free(line);
        free(args);
    } while (status);
    free(name);
}

// read line function
char *nash_read_line(void)
{
    int bufsize = NASH_RL_BUFSIZE;
    int position = 0;
    char *buffer = (char *)std::malloc(sizeof(char) * bufsize); // assign an initial buffer
    int c;                                                      // "int" to detect EOF

    if (!buffer)
    {
        std::cerr << "nash: allocation error\n";
        std::exit(EXIT_FAILURE);
    }

    while (1)
    {
        c = getchar();

        if (c == '\f') // CTRL + L --> clear screen
        {
            std::cout << "\033[H\033[2J";
            std::cout << YELLOW << name << RESET << " @ " << GREEN << wk_dir << RESET << " > ";
            buffer[position] = '\0';
            std::cout << buffer;
            std::cout.flush();
            buffer[position] = ' ';
            continue;
        }
        else if (c == EOF || c == '\n')
        {
            std::cout << '\n';
            std::cout.flush();
            buffer[position] = '\0';
            return buffer;
        }
        else if (c == 127 || c == '\b')
        {
            if (position != 0)
            {
                position--;
                std::cout << "\b \b";
                std::cout.flush();
            }

            continue;
        }
        else
        {
            buffer[position] = c;
            std::cout << (char)c;
            std::cout.flush();
            position++;
        }

        // if exceeded allocated buffer, allocate more memory
        if (position >= bufsize)
        {
            bufsize += NASH_RL_BUFSIZE; // double the size
            buffer = (char *)std::realloc(buffer, bufsize);
            if (!buffer)
            {
                std::cerr << "nash: allocation error\n";
                std::exit(EXIT_FAILURE);
            }
        }
    }
}

// split line function
char **nash_split_line(char *line)
{
    int bufsize = NASH_TOK_BUFSIZE, position = 0;
    char **tokens = (char **)std::malloc(sizeof(char *) * bufsize);
    char *token;

    if (!tokens)
    {
        std::cerr << "nash: allocation error\n";
        std::exit(EXIT_FAILURE);
    }

    token = strtok(line, NASH_TOK_DELIM);
    while (token != nullptr)
    {
        tokens[position] = token;
        position++;

        if (position >= bufsize)
        {
            bufsize += NASH_TOK_BUFSIZE;
            tokens = (char **)std::realloc(tokens, sizeof(char *) * bufsize);
            if (!tokens)
            {
                std::cerr << "nash: allocation error\n";
                std::exit(EXIT_FAILURE);
            }
        }

        token = strtok(nullptr, NASH_TOK_DELIM);
    }

    tokens[position] = nullptr;
    return tokens;
}

// launch process function
int nash_launch(char **args)
{
    pid_t pid, wpid;
    int status;

    pid = fork();
    if (pid == 0)
    {
        // child process
        if (execvp(args[0], args) == -1)
        {
            perror("nash");
        }
        exit(EXIT_FAILURE);
    }
    else if (pid < 0)
    {
        perror("nash");
    }
    else
    {
        // parent process
        do
        {
            wpid = waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }

    return 1;
}

// main execution function
int nash_execute(char **args)
{
    if (args[0] == nullptr)
    {
        return 1;
    }

    if (process_pipe(args))
    {
        return 1;
    }

    for (int i = 0; i < nash_builtins_nums(); i++)
    {
        if (strcmp(args[0], nash_builtins_str[i]) == 0)
        {
            return (*nash_builtins_func[i])(args);
        }
    }

    return nash_launch(args);
}

// main function
int main(int argc, char **argv)
{
    // save original terminal
    tcgetattr(STDIN_FILENO, &original_termios);

    // create new terminal
    raw = original_termios;
    set_ncanonical_mode();

    // setup signal handlers
    install_signal_handlers();

    // run shell loop
    nash_loop();

    // reset terminal
    restore_terminal();

    // exit program
    return EXIT_SUCCESS;
}