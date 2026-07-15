#pragma once

#include <parser.h>

#include <vector>

// piping logic declaration
int process_pipe(char **args);

// shell launch declaration
int nash_launch(char **args);

// shell execute binaries declaration
int nash_execute(std::vector<Command> &commands);

// output redirection declaration
void setup_output_redirection(Command &cmd);

// input redirection declaration
void setup_input_redirection(Command &cmd);

// execute commands declaration
int execute_commands(std::vector<Command> &commands);
