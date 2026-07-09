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
    install_signal_handlers();

    // run shell loop
    Shell shell;
    shell.run();

    // reset terminal
    restore_terminal();

    // exit program
    return EXIT_SUCCESS;
}