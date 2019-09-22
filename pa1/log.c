#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

#include "ipc.h"
#include "pa1.h"


void log_started(FILE *events_log_fd, local_id process_id) {
    fprintf(events_log_fd, log_started_fmt, process_id, getpid(), getppid());
}

void log_received_all_started(FILE *events_log_fd, local_id process_id) {
    fprintf(events_log_fd, log_received_all_started_fmt, process_id);
}

void log_done(FILE *events_log_fd, local_id process_id) {
    fprintf(events_log_fd, log_done_fmt, process_id);
}

void log_received_all_done(FILE *events_log_fd, local_id process_id) {
    fprintf(events_log_fd, log_received_all_done_fmt, process_id);
}

void log_pipe_created(FILE *pipes_log_file, int from, int to, int read_fd, int write_fd) {
    fprintf(pipes_log_file, "Created pipe from process %2d to %2d with read_fd %2d and write_fd %2d.\n",
            from, to, read_fd, write_fd);
}

void fatalf(char const *const fmt, ...) {
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
