#pragma once

#include <vector>
#include <string>

// command struct
struct Command
{
    // command args list
    std::vector<char *> args;

    // file paths
    char *input_file = nullptr;
    char *output_file = nullptr;

    // useful flags
    bool has_input = false;
    bool has_output = false;
    bool append = false;
    bool background = false;
};

// command parsing declartion
std::vector<Command> parse_commands(char **args);

// shell read line declaration
char *nash_read_line();

// shell tokenize line declaration
char **nash_split_line(char *line);
