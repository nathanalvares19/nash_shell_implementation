#include "builtins.h"
#include "constants.h"
#include "executor.h"
#include "terminal.h"
#include "shell.h"
#include "globals.h"

#include <iostream>

int main(int argc, char **argv)
{
    // initialize terminal
    initialize_terminal();

    // setup signal handlers
    shell.init_signals();

    // run shell loop
    shell.run();

    // reset terminal
    restore_terminal();

    // exit program
    return EXIT_SUCCESS;
}