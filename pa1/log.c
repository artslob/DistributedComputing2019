#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

#include "ipc.h"
#include "pa1.h"


void log_started(int events_log_fd, local_id process_id) {
    int BUF_LEN = 75;
    char buf[BUF_LEN];
    snprintf(buf, BUF_LEN, log_started_fmt, process_id, getpid(), getppid());
    write(events_log_fd, buf, strlen(buf));
}

void log_received_all_started(int events_log_fd, local_id process_id) {
    int BUF_LEN = 75;
    char buf[BUF_LEN];
    snprintf(buf, BUF_LEN, log_received_all_started_fmt, process_id);
    write(events_log_fd, buf, strlen(buf));
}

void log_done(int events_log_fd, local_id process_id) {
    int BUF_LEN = 75;
    char buf[BUF_LEN];
    snprintf(buf, BUF_LEN, log_done_fmt, process_id);
    write(events_log_fd, buf, strlen(buf));
}

void log_received_all_done(int events_log_fd, local_id process_id) {
    int BUF_LEN = 75;
    char buf[BUF_LEN];
    snprintf(buf, BUF_LEN, log_received_all_done_fmt, process_id);
    write(events_log_fd, buf, strlen(buf));
}

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
