#include "globals.h"
#include "constants.h"

#include <iostream>

char *name = nullptr;
char *wk_dir = (char *)std::malloc(sizeof(char) * WDIR_BUFSIZE);

History history;
Shell shell;