#include "parser.h"
#include "constants.h"
#include "globals.h"

#include <iostream>
#include <cstdlib>
#include <cstring>

// read line function
char *nash_read_line(void)
{
    int bufsize = NASH_RL_BUFSIZE;
    int position = 0;
    char *buffer = (char *)std::malloc(sizeof(char) * bufsize); // assign an initial buffer
    int c;                                                      // "int" to detect EOF

    if (!buffer)
    {
        std::cerr << "nash: allocation error\n";
        std::exit(EXIT_FAILURE);
    }

    while (1)
    {
        c = getchar();

        if (c == '\f') // CTRL + L --> clear screen
        {
            std::cout << "\033[H\033[2J";
            std::cout << Colours::Yellow << name << Colours::Reset << " @ " << Colours::Green << wk_dir << Colours::Reset << " > ";
            buffer[position] = '\0';
            std::cout << buffer;
            std::cout.flush();
            buffer[position] = ' ';
            continue;
        }
        else if (c == EOF || c == '\n')
        {
            std::cout << '\n';
            std::cout.flush();
            buffer[position] = '\0';
            return buffer;
        }
        else if (c == 127 || c == '\b')
        {
            if (position != 0)
            {
                position--;
                std::cout << "\b \b";
                std::cout.flush();
            }

            continue;
        }
        else
        {
            buffer[position] = c;
            std::cout << (char)c;
            std::cout.flush();
            position++;
        }

        // if exceeded allocated buffer, allocate more memory
        if (position >= bufsize)
        {
            bufsize += NASH_RL_BUFSIZE; // double the size
            buffer = (char *)std::realloc(buffer, bufsize);
            if (!buffer)
            {
                std::cerr << "nash: allocation error\n";
                std::exit(EXIT_FAILURE);
            }
        }
    }
}

// tokenize line function
char **nash_split_line(char *line)
{
    int bufsize = NASH_TOK_BUFSIZE, position = 0;
    char **tokens = (char **)std::malloc(sizeof(char *) * bufsize);
    char *token;

    if (!tokens)
    {
        std::cerr << "nash: allocation error\n";
        std::exit(EXIT_FAILURE);
    }

    token = strtok(line, NASH_TOK_DELIM);
    while (token != nullptr)
    {
        tokens[position] = token;
        position++;

        if (position >= bufsize)
        {
            bufsize += NASH_TOK_BUFSIZE;
            tokens = (char **)std::realloc(tokens, sizeof(char *) * bufsize);
            if (!tokens)
            {
                std::cerr << "nash: allocation error\n";
                std::exit(EXIT_FAILURE);
            }
        }

        token = strtok(nullptr, NASH_TOK_DELIM);
    }

    tokens[position] = nullptr;
    return tokens;
}