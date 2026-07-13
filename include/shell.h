#pragma once

class Shell
{
public:
    void run();
    void prompt(char *name);

private:
    char *process_name();
};
