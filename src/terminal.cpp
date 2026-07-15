#include "terminal.h"
#include "globals.h"

#include <unistd.h>
#include <termios.h>

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
