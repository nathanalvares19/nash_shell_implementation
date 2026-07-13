#include "parser.h"
#include "constants.h"
#include "globals.h"
#include "builtins.h"

#include <iostream>
#include <cstdlib>
#include <cstring>

// redraw function
void redraw_line(const char *buffer, int length, int cursor)
{
    std::cout << '\r';
    shell.prompt(name);

    for (int i = 0; i < length; i++)
    {
        std::cout << buffer[i];
    }

    std::cout << "\033[K"; // erase from cursor to end of line

    int move_left = length - cursor;
    if (move_left > 0)
    {
        std::cout << "\033[" << move_left << "D";
    }

    std::cout.flush();
}

// read line function
char *nash_read_line(void)
{
    int bufsize = NASH_RL_BUFSIZE;
    int length = 0;
    int cursor = 0;
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

        // arrow keys
        if (c == 27)
        {
            char seq1 = getchar();
            char seq2 = getchar();

            if (seq1 == '[')
            {
                if (seq2 == 'A') // up
                {
                    // do nothing
                }
                else if (seq2 == 'B') // down
                {
                    // do nothing
                }
                else if (seq2 == 'C') // right
                {
                    if (cursor < length)
                    {
                        cursor++;
                    }
                    redraw_line(buffer, length, cursor);
                }
                else if (seq2 == 'D') // left
                {
                    if (cursor > 0)
                    {
                        cursor--;
                    }
                    redraw_line(buffer, length, cursor);
                }
            }
            continue;
        }

        if (c == '\f') // CTRL + L --> clear screen
        {
            std::cout << "\033[H\033[2J";
            redraw_line(buffer, length, cursor);
            continue;
        }
        else if (c == EOF || c == '\n')
        {
            std::cout << '\n';
            std::cout.flush();
            buffer[length] = '\0';
            return buffer;
        }
        else if (c == 127 || c == '\b')
        {
            if (cursor > 0 && length > 0)
            {
                cursor--;
                length--;

                // shift buffer to left
                for (int i = cursor; i < length; i++)
                {
                    buffer[i] = buffer[i + 1];
                }

                redraw_line(buffer, length, cursor);
            }

            continue;
        }
        else
        {
            for (int i = length; i >= cursor; i--)
            {
                buffer[i] = buffer[i - 1];
            }

            buffer[cursor] = c;
            cursor++;
            length++;

            redraw_line(buffer, length, cursor);
        }

        // if exceeded allocated buffer, allocate more memory
        if (length >= bufsize)
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

// read line function
// char *nash_read_line(void)
// {
//     int bufsize = NASH_RL_BUFSIZE;
//     int position = 0;
//     char *buffer = (char *)std::malloc(sizeof(char) * bufsize); // assign an initial buffer
//     int c;                                                      // "int" to detect EOF

//     if (!buffer)
//     {
//         std::cerr << "nash: allocation error\n";
//         std::exit(EXIT_FAILURE);
//     }

//     while (1)
//     {
//         c = getchar();

//         if (c == '\f') // CTRL + L --> clear screen
//         {
//             std::cout << "\033[H\033[2J";
//             shell.prompt(name);
//             buffer[position] = '\0';
//             std::cout << buffer;
//             std::cout.flush();
//             buffer[position] = ' ';
//             continue;
//         }
//         else if (c == EOF || c == '\n')
//         {
//             std::cout << '\n';
//             std::cout.flush();
//             buffer[position] = '\0';
//             return buffer;
//         }
//         else if (c == 127 || c == '\b')
//         {
//             if (position != 0)
//             {
//                 position--;
//                 std::cout << "\b \b";
//                 std::cout.flush();
//             }

//             continue;
//         }
//         else
//         {
//             buffer[position] = c;
//             std::cout << (char)c;
//             std::cout.flush();
//             position++;
//         }

//         // if exceeded allocated buffer, allocate more memory
//         if (position >= bufsize)
//         {
//             bufsize += NASH_RL_BUFSIZE; // double the size
//             buffer = (char *)std::realloc(buffer, bufsize);
//             if (!buffer)
//             {
//                 std::cerr << "nash: allocation error\n";
//                 std::exit(EXIT_FAILURE);
//             }
//         }
//     }
// }

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