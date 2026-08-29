#include "StdoutRedirect.h"

#ifndef _WIN32
#include <unistd.h>
#include <cstdio>
#endif

StdoutRedirect::StdoutRedirect(bool enabled) {
#ifndef _WIN32
    if (enabled) {
        fflush(stdout);
        savedStdoutFd = dup(STDOUT_FILENO);
        dup2(STDERR_FILENO, STDOUT_FILENO);
    }
#endif
}

StdoutRedirect::~StdoutRedirect() {
    restore();
}

void StdoutRedirect::restore() {
#ifndef _WIN32
    if (savedStdoutFd >= 0) {
        fflush(stderr);
        dup2(savedStdoutFd, STDOUT_FILENO);
        close(savedStdoutFd);
        savedStdoutFd = -1;
    }
#endif
}
