/*
 * ============================================================================
 * File        : main.cpp
 * Project     : Nash Shell
 * Author      : Nathan Alvares
 * Description : A Unix-like shell built from scratch in C++ using POSIX system
 *               calls. The project is intended for learning.
 * ============================================================================
 */

#include <iostream>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <sys/wait.h>
#include <string>

/*
    MACROS
*/

// read line function
#define NASH_RL_BUFSIZE 1024

// split line function
#define NASH_TOK_BUFSIZE 64
#define NASH_TOK_DELIM " \t\r\n\a"

// version number
#define NASH_VERSION "1.0.0"

// username size
#define USERNAME_BUFSIZE 32

// username size
#define WDIR_BUFSIZE 128

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
int nash_builtins_nums();

/*
    FUNCTION DEFINITIONS
*/

// working directory variable
char *wk_dir = (char *)std::malloc(sizeof(char) * WDIR_BUFSIZE);

// built-in functions
const char *nash_builtins_str[] = {
    "cd",
    "help",
    "exit"};

int (*nash_builtins_func[])(char **) = {
    nash_cd,
    nash_help,
    nash_exit};

int nash_builtins_nums()
{
    return sizeof(nash_builtins_str) / sizeof(const char *);
}

// CHANGE DIRECTORY FUNCTION
int nash_cd(char **args)
{
    if (args[1] == nullptr)
    {
        std::cerr << "nash: expected argument for \"cd\"\n";
    }
    else
    {
        if (chdir(args[1]) != 0)
        {
            perror("nash");
        }

        if (getcwd(wk_dir, WDIR_BUFSIZE) == nullptr)
        {
            perror("getcwd");
        }
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
        std::cout << (i + 1) << ". " << nash_builtins_str[i] << "\n";
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

// main shell loop
void nash_loop(void)
{
    char *line;
    char **args;
    int status;

    // username implementation
    char *name = (char *)std::malloc(sizeof(char) * USERNAME_BUFSIZE);
    if (!name)
    {
        std::cerr << "nash: allocation error\n";
        std::exit(EXIT_FAILURE);
    }

    size_t bufsize = USERNAME_BUFSIZE;
    std::cout << "Enter username (maximum 32 chars): ";
    size_t name_length = getline(&name, &bufsize, stdin);

    if (name_length == -1)
    {
        free(name);
        return;
    }
    else
    {
        if (name[name_length - 1] == '\n')
        {
            name[name_length - 1] = '\0';
        }
    }

    if (getcwd(wk_dir, WDIR_BUFSIZE) == nullptr)
    {
        perror("getcwd");
    }

    do
    {
        std::cout << name << ": " << wk_dir << " > ";
        line = nash_read_line();
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

        if (c == EOF || c == '\n')
        {
            buffer[position] = '\0';
            return buffer;
        }
        else
        {
            buffer[position] = c;
        }
        position++;

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
    nash_loop();
    return EXIT_SUCCESS;
}