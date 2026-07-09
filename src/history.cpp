#include "history.h"

#include <cstring>
#include <iostream>
#include <iomanip>

// add to history function
void History::add(const char *line)
{
    History::last_insert_idx = (History::last_insert_idx + 1) % HIST_BUFSIZE;
    History::count += 1;

    if (History::count > 128)
    {
        free(History::lines[History::last_insert_idx]);
    }

    char *cmd = strdup(line);
    if (cmd == nullptr)
    {
        perror("strdup");
        return;
    }
    History::lines[History::last_insert_idx] = cmd;
}

// print history function
void History::print() const
{
    if (History::count > 128)
    {
        int idx = (History::last_insert_idx + 1) % HIST_BUFSIZE;
        int num = 1;
        while (idx != History::last_insert_idx)
        {
            std::cout << "  " << std::setw(3) << num << "  " << History::lines[idx] << "\n";
            idx = (idx + 1) % HIST_BUFSIZE;
            num += 1;
        }
        std::cout << "  " << std::setw(3) << num << "  " << History::lines[idx] << "\n";
    }
    else
    {
        for (int i = 0; i <= History::last_insert_idx; i++)
        {
            std::cout << "  " << std::setw(3) << (i + 1) << "  " << History::lines[i] << "\n";
        }
    }
}