#include "executor.h"
#include "builtins.h"
#include "globals.h"
#include "terminal.h"

#include <fcntl.h>
#include <cstring>
#include <unistd.h>
#include <sys/wait.h>
#include <iostream>
#include <vector>

// input redirection
int input_redirection(char **args)
{
    // find operator
    int flag = 0;
    char *input_file;

    for (int i = 0; args[i] != nullptr; i++)
    {
        if (strcmp(args[i], "<") == 0)
        {
            flag = 1;
            args[i] = nullptr;
            input_file = args[i + 1];
            break;
        }
    }

    if (!flag)
    {
        return 0;
    }
    else if (input_file != nullptr)
    {
        // processing
        int fd = open(input_file, O_RDONLY);

        if (fd < 0)
        {
            perror("nash");
            return 1;
        }

        pid_t pid = fork();
        if (pid == 0)
        {
            if (dup2(fd, STDIN_FILENO) == -1)
            {
                perror("nash");
                exit(EXIT_FAILURE);
            }

            close(fd);

            if (execvp(args[0], args) == -1)
            {
                perror("nash");
                exit(EXIT_FAILURE);
            }
        }

        close(fd);
        waitpid(pid, nullptr, 0);
    }
    return 1;
}

// output redirection
int output_redirection(char **args)
{
    // find operator
    int flag = 0;
    char *output_file;

    for (int i = 0; args[i] != nullptr; i++)
    {
        if (strcmp(args[i], ">") == 0)
        {
            flag = 1;
            args[i] = nullptr;
            output_file = args[i + 1];
            break;
        }
    }

    if (!flag)
    {
        return 0;
    }
    else if (output_file != nullptr)
    {
        // processing
        int fd = open(output_file, O_CREAT | O_WRONLY | O_TRUNC, 0644);

        if (fd < 0)
        {
            perror("nash");
            return 1;
        }

        pid_t pid = fork();
        if (pid == 0)
        {
            if (dup2(fd, STDOUT_FILENO) == -1)
            {
                perror("nash");
                exit(EXIT_FAILURE);
            }

            close(fd);

            if (execvp(args[0], args) == -1)
            {
                perror("nash");
                exit(EXIT_FAILURE);
            }
        }

        close(fd);
        waitpid(pid, nullptr, 0);
    }
    return 1;
}

// multiple pipes processing
int process_pipe(char **args)
{
    // find pipe
    std::vector<int> pipe_indexes;
    for (int i = 0; args[i] != nullptr; i++)
    {
        if (strcmp(args[i], "|") == 0)
        {
            // adjacent pipes
            if (!pipe_indexes.empty() && i == pipe_indexes.back() + 1)
            {
                return 0;
            }
            pipe_indexes.push_back(i);
        }
    }

    // pipe existence check
    if (pipe_indexes.empty())
    {
        return 0;
    }
    else if (pipe_indexes[0] == 0 || args[pipe_indexes.back() + 1] == nullptr) // invalid pipe usage cehck
    {
        std::cerr << "nash: invalid pipe usage\n";
        return 0;
    }

    // split args around pipe
    int num_of_pipes = pipe_indexes.size();
    std::vector<char **> pipe_args;
    pipe_args.push_back(args);
    for (int i = 0; i < num_of_pipes; i++)
    {
        args[pipe_indexes[i]] = nullptr;
        pipe_args.push_back(&args[pipe_indexes[i] + 1]);
    }

    // create pipes
    std::vector<std::vector<int>> pipes(num_of_pipes, std::vector<int>(2));
    for (int i = 0; i < num_of_pipes; i++)
    {
        if (pipe(pipes[i].data()) == -1)
        {
            perror("pipe");
            return 1;
        }
    }

    // create children
    std::vector<pid_t> pids; // store child pids
    for (int i = 0; i < num_of_pipes + 1; i++)
    {
        pid_t pid = fork();

        // error check
        if (pid < 0)
        {
            perror("nash");
            return 1;
        }

        // store child pids
        if (pid > 0)
        {
            pids.push_back(pid);
        }

        // first cmd
        if (i == 0 && pid == 0)
        {
            dup2(pipes[i][1], STDOUT_FILENO); // write end

            // close child fds
            for (int j = 0; j < num_of_pipes; j++)
            {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }

            if (execvp(pipe_args[i][0], pipe_args[i]) == -1)
            {
                perror("nash");
                exit(EXIT_FAILURE);
            }
        }

        // last cmd
        if (i == num_of_pipes && pid == 0)
        {
            dup2(pipes[i - 1][0], STDIN_FILENO); // read end

            // close child fds
            for (int j = 0; j < num_of_pipes; j++)
            {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }

            if (execvp(pipe_args[i][0], pipe_args[i]) == -1)
            {
                perror("nash");
                exit(EXIT_FAILURE);
            }
        }

        // middle cmds
        if ((i != 0 && i != num_of_pipes) && pid == 0)
        {
            dup2(pipes[i - 1][0], STDIN_FILENO); // read end
            dup2(pipes[i][1], STDOUT_FILENO);    // write end

            // close child fds
            for (int j = 0; j < num_of_pipes; j++)
            {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }

            if (execvp(pipe_args[i][0], pipe_args[i]) == -1)
            {
                perror("nash");
                exit(EXIT_FAILURE);
            }
        }
    }

    // close parent pipe file descriptors
    for (int i = 0; i < num_of_pipes; i++)
    {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }

    // parent waits for children
    for (auto pid : pids)
    {
        waitpid(pid, nullptr, 0);
    }

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
        restore_terminal();
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

    set_ncanonical_mode();

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

    if (output_redirection(args))
    {
        return 1;
    }

    if (input_redirection(args))
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