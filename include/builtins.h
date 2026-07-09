#pragma once

// built-in function declarations
int nash_cd(char **args);
int nash_help(char **args);
int nash_exit(char **args);
int nash_pwd(char **args);
int nash_clear(char **args);
int nash_history(char **args);

// built-in function count declaration
int nash_builtins_nums();

// variable and data structure declarations
extern const char *nash_builtins_str[];
extern const char *nash_builtins_desc[];
extern int (*nash_builtins_func[])(char **);