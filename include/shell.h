#pragma once

#include <vector>
#include <string>
#include "jobs.h"

class Shell
{
public:
    void prompt(char *name);
    void run();
    void add_job(pid_t pid, const std::string &command);
    void print_jobs();
    void reap_jobs();

    int next_job_id = 1;

private:
    std::vector<Job> jobs;
    char *process_name();
};
