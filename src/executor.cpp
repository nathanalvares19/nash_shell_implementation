#include "executor.h"
#include "builtins.h"
#include "globals.h"

#include <cstring>
#include <unistd.h>
#include <sys/wait.h>
#include <iostream>

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