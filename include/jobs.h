#pragma once

#include <string>
#include <sys/types.h>

struct Job
{
    int id;
    pid_t pid;
    std::string command;
};
