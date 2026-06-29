#include <iostream>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <sys/wait.h>
// read line function
#define NASH_RL_BUFSIZE 1024

// split line function
#define NASH_TOK_BUFSIZE 64
#define NASH_TOK_DELIM " \t\r\n\a"

// main shell loop
void nash_loop(void)
{
    char *line;
    char **args;
    int status;

    do
    {
        std::cout << "> ";
        line = nash_read_line();
        args = nash_split_line(line);
        status = nash_execute(args);

        free(line);
        free(args);

    } while (status);
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
            std::realloc(tokens, sizeof(char *) * bufsize);
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

int nash_cd(char **args);
int nash_help(char **args);
int nash_exit(char **args);

// built-in functions
char *nash_builtins_str[]{
    "cd",
    "help",
    "exit"};

int (*nash_builtins_func[])(char **) = {
    &nash_cd,
    &nash_help,
    &nash_exit}

int nash_builtins_nums()
{
    return sizeof(nash_builtins_str) / sizeof(char *);
}

// CHANGE DIRECTORY FUNCTION
int nash_cd(char **args)
{
    if (args[1] == nullptr)
    {
        std::cerr << "nash: expected argument to \"cd\"\n";
    }
    else
    {
        if (chdir(args[1]) != 0)
        {
            perror("nash");
        }
    }
    return 1;
}

// HELP FUNCTION
int nash_help(char **args)
{
    std::cout << "Nathan's NASH Shell\n";
    std::cout << "Type program names and arguments, and hit enter.\n";
    std::cout << "The following are built-in:\n";

    for (int i = 0; i < nash_builtins_nums(); i++)
    {
        std::cout << nash_builtins_str[i] << "\n";
    }

    return 1;
}

// EXIT FUNCTION
int nash_exit(char **args)
{
    return 0;
}

int nash_execute(char **args)
{
    if (args[0] == nullptr)
    {
        return 1;
    }

    for (int i = 0; i < nash_builtins_nums(); i++)
    {
        if (strcmp(args[0], nash_builtins_str[i]) != 0)
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