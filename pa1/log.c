#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>


void log_pipe_created(int pipes_log_fd, int from, int to, int read_fd, int write_fd) {
    int BUF_LEN = 75;
    char buf[BUF_LEN];
    snprintf(buf, BUF_LEN, "Created pipe from process %2d to %2d with read_fd %2d and write_fd %2d.\n",
                            from, to, read_fd, write_fd);
    write(pipes_log_fd, buf, strlen(buf));
}

void fatalf(char const * const fmt, ...) {
    // define pointer on argument list
    va_list ptr;
    // init pointer so it will be pointing on first arg after `fmt` arg
    va_start(ptr, fmt);
    // use special version of `printf`
    vprintf(fmt, ptr);
    // clean memory
    va_end(ptr);
    exit(1);
}
