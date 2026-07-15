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

// execute commands function
int execute_commands(std::vector<Command> &commands)
{
    // pipe creation
    int num_of_pipes = commands.size() - 1;
    int pipes[num_of_pipes][2];

    for (int i = 0; i < num_of_pipes; i++)
    {
        if (pipe(pipes[i]) == -1)
        {
            perror("nash");
            return 1;
        }
    }

    // create child processes
    std::vector<pid_t> pids;
    for (int i = 0; i < commands.size(); i++)
    {
        pid_t pid = fork();

        if (pid < 0)
        {
            perror("nash");
            return 1;
        }

        // child process
        if (pid == 0)
        {
            // pipe setup
            if (i != 0)
            {
                dup2(pipes[i - 1][0], STDIN_FILENO); // input
            }

            if (i != commands.size() - 1)
            {
                dup2(pipes[i][1], STDOUT_FILENO); // output
            }

            // close child pipe fds
            for (int i = 0; i < num_of_pipes; i++)
            {
                close(pipes[i][0]);
                close(pipes[i][1]);
            }

            // redirection setup
            if (commands[i].has_input)
            {
                setup_input_redirection(commands[i]);
            }

            if (commands[i].has_output)
            {
                setup_output_redirection(commands[i]);
            }

            if (execvp(commands[i].args[0], commands[i].args.data()) == -1)
            {
                perror("nash");
                exit(EXIT_FAILURE);
            }
        }
        else
        {
            // parent
            pids.push_back(pid);
        }
    }

    // parent process
    // close parent pipe fds
    for (int i = 0; i < num_of_pipes; i++)
    {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }

    // wait for children
    if (!commands[0].background)
    {
        for (pid_t pid : pids)
        {
            waitpid(pid, nullptr, 0);
        }
    }
    else
    {
        std::cout << "pid: " << pids[0] << "\n";
    }

    return 1;
}

// input redirection
void setup_input_redirection(Command &cmd)
{
    if (cmd.input_file != nullptr)
    {
        // processing
        int fd = open(cmd.input_file, O_RDONLY);

        if (fd < 0)
        {
            perror("nash");
            return;
        }

        if (dup2(fd, STDIN_FILENO) == -1)
        {
            perror("nash");
            return;
        }

        close(fd);
    }
}

// output redirection
void setup_output_redirection(Command &cmd)
{
    if (cmd.output_file != nullptr)
    {
        int fd;

        // append logic
        if (cmd.append)
        {
            fd = open(cmd.output_file, O_CREAT | O_WRONLY | O_APPEND, 0644);
        }
        else
        {
            fd = open(cmd.output_file, O_CREAT | O_WRONLY | O_TRUNC, 0644);
        }

        if (fd < 0)
        {
            perror("nash");
            return;
        }

        if (dup2(fd, STDOUT_FILENO) == -1)
        {
            perror("nash");
            return;
        }

        close(fd);
    }
}

// for general processes
int nash_launch(std::vector<Command> &commands)
{
    pid_t pid;
    int status;

    pid = fork();

    if (pid < 0)
    {
        perror("nash");
        return 1;
    }

    // child process
    if (pid == 0)
    {
        restore_terminal();
        if (execvp(commands[0].args[0], commands[0].args.data()) == -1)
        {
            perror("nash");
        }
        exit(EXIT_FAILURE);
    }
    else
    {
        // parent process
        if (!commands[0].background)
        {
            do
            {
                waitpid(pid, &status, WUNTRACED);
            } while (!WIFEXITED(status) && !WIFSIGNALED(status));
        }
        else
        {
            std::cout << "pid: " << pid << "\n";
        }
    }

    set_ncanonical_mode();

    return 1;
}

// main execution function
int nash_execute(std::vector<Command> &commands)
{
    if (commands.empty())
    {
        return 1;
    }

    // builtins
    for (int i = 0; i < nash_builtins_nums(); i++)
    {
        if (strcmp(commands[0].args[0], nash_builtins_str[i]) == 0)
        {
            return (*nash_builtins_func[i])(commands[0].args.data());
        }
    }

    // |, <, >, >> processing and execution
    if (commands.size() > 1 || commands[0].has_input || commands[0].has_output)
    {
        return execute_commands(commands);
    }

    // other processes
    return nash_launch(commands);
}