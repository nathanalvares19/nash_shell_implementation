#include "terminal.h"
#include "globals.h"

#include <unistd.h>
#include <termios.h>
#include <signal.h>

struct termios original_termios;
struct termios raw;

// initialize terminal
void initialize_terminal()
{
    tcgetattr(STDIN_FILENO, &original_termios);

    raw = original_termios;

    set_ncanonical_mode();
}

// disable canonical mode
void set_ncanonical_mode()
{
    raw.c_lflag &= ~ICANON; // disable canonical mode
    raw.c_lflag &= ~ECHO;   // disable echo
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

// reset canonical mode
void restore_terminal()
{
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &original_termios);
}

// SIGINT handler
void sigint_handler(int sig)
{
    write(STDOUT_FILENO, "^C", 2);
    return;
}

// setup signal handlers
void install_signal_handlers()
{
    struct sigaction sa{};

    sa.sa_handler = sigint_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, nullptr);
}
