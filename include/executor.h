#pragma once

// piping logic declaration
int process_pipe(char **args);

// shell launch declaration
int nash_launch(char **args);

// shell execute binaries declaration
int nash_execute(char **args);

// output redirection declaration
int output_redirection(char **args);
