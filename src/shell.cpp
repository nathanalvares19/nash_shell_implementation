#include "shell.h"
#include "executor.h"
#include "constants.h"
#include "globals.h"
#include "parser.h"

#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include <cstring>

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
