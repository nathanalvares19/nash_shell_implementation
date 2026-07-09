#pragma once

// initialize terminal declaration
void initialize_terminal();

// terminal writing mode declarations
void set_ncanonical_mode();
void restore_terminal();

// signal handling declarations
void install_signal_handlers();
