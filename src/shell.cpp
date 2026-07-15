#include "shell.h"
#include "executor.h"
#include "constants.h"
#include "globals.h"
#include "parser.h"

#include <signal.h>
#include <iomanip>
#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include <cstring>
#include <sys/wait.h>
#include <algorithm>

// print shell prompt
void Shell::prompt(char *name)
{
    std::cout << Colours::Yellow << name << Colours::Reset << " @ " << Colours::Green << wk_dir << Colours::Reset << " > ";
}

// main shell loop
void Shell::run()
{
    char *line;
    char **args;
    int status;
    std::vector<Command> commands;

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
        // remove child processes from jobs table
        reap_jobs();

        // prompt
        prompt(name);
        line = nash_read_line();
        if (strlen(line) != 0) // only add to history if not empty
        {
            history.add(line);
        }
        args = nash_split_line(line);
        commands = parse_commands(args);
        status = nash_execute(commands);

        free(line);
        free(args);
    } while (status);
    free(name);
}

// username processing
char *Shell::process_name()
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

// add job function
void Shell::add_job(pid_t pid, const std::string &command)
{
    jobs.push_back({next_job_id++,
                    pid,
                    command});
}

// print jobs
void Shell::print_jobs()
{

    if (!jobs.empty())
    {
        std::cout << std::left << std::setw(6) << "JOB" << std::setw(10) << "PID" << "COMMAND\n";

        std::cout << "------------------------\n";
    }
    else
    {
        std::cout << "No active jobs\n";
    }

    for (const auto &job : jobs)
    {

        std::cout << std::left << std::setw(6) << ("[" + std::to_string(job.id) + "]") << std::setw(10) << job.pid << job.command << '\n';
    }
}

// reap jobs - basically just delete from the vector
void Shell::reap_jobs()
{
    pid_t pid;

    while ((pid = waitpid(-1, nullptr, WNOHANG)) > 0)
    {
        auto it = std::find_if(jobs.begin(), jobs.end(), [pid](const Job &job)
                               { return job.pid == pid; });

        if (it != jobs.end())
        {
            std::cout << "\nJob finished: " << it->id << " " << it->pid << "\n";
            jobs.erase(it);
        }
    }
}

// signal handling
void Shell::init_signals()
{
    struct sigaction sa;

    // register SIGINT handler
    sa.sa_handler = Shell::handle_sigint;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, nullptr);

    // register SIGTSTP handler
    sa.sa_handler = SIG_IGN;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGTSTP, &sa, nullptr);
}

// SIGINT handler
void Shell::handle_sigint(int sig)
{
    write(STDOUT_FILENO, "SIGINT triggered", 17);
    return;
}
