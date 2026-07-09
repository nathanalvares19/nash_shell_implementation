#pragma once

class History
{
public:
    void add(const char *line);
    void print() const;

private:
    static constexpr int HIST_BUFSIZE = 128;
    char *lines[HIST_BUFSIZE]{};

    int last_insert_idx = -1;
    int count = 0;
};