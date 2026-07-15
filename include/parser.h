#pragma once

#include <vector>
#include <string>

// command struct
struct Command
{
    std::vector<char *> args;

    char *input_file;
    char *output_file;

    bool has_input = false;
    bool has_output = false;
    bool append = false;
};

// command parsing declartion
std::vector<Command> parse_commands(char **args);

// shell read line declaration
char *nash_read_line();

// shell tokenize line declaration
char **nash_split_line(char *line);
