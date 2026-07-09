#pragma once

// colour support
namespace Colours
{
    constexpr const char *Black = "\033[30m";
    constexpr const char *Red = "\033[31m";
    constexpr const char *Green = "\033[32m";
    constexpr const char *Yellow = "\033[33m";
    constexpr const char *Blue = "\033[34m";
    constexpr const char *Magenta = "\033[35m";
    constexpr const char *Cyan = "\033[36m";
    constexpr const char *White = "\033[37m";
    constexpr const char *Reset = "\033[0m";
}

// read line function
constexpr int NASH_RL_BUFSIZE = 1024;

// split line function
constexpr int NASH_TOK_BUFSIZE = 64;
constexpr const char *NASH_TOK_DELIM = " \t\r\n\a";

// username size
constexpr int USERNAME_BUFSIZE = 32;

// working directory size
constexpr int WDIR_BUFSIZE = 128;

// history size
constexpr int HIST_BUFSIZE = 128;