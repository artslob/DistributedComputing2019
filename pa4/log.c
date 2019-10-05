#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <assert.h>

#include "ipc.h"
#include "pa2345.h"
#include "log.h"
#include "lamport.h"


void log_started(FILE *events_log_file, local_id process_id) {
    fprintf(events_log_file, log_started_fmt, lamport_get_time(), process_id, getpid(), getppid(), 0);
}

void log_received_all_started(FILE *events_log_file, local_id process_id) {
    fprintf(events_log_file, log_received_all_started_fmt, lamport_get_time(), process_id);
}

void log_done(FILE *events_log_file, local_id process_id) {
    fprintf(events_log_file, log_done_fmt, lamport_get_time(), process_id, 0);
}

void log_received_all_done(FILE *events_log_file, local_id process_id) {
    fprintf(events_log_file, log_received_all_done_fmt, lamport_get_time(), process_id);
}

static const char *const log_pipe_created_fmt =
        "Created pipe from process %d to %d with read_fd %2d and write_fd %2d.\n";

void log_pipe_created(FILE *pipes_log_file, local_id from, local_id to, int read_fd, int write_fd) {
    fprintf(pipes_log_file, log_pipe_created_fmt, from, to, read_fd, write_fd);
}

void log_loop_operation(local_id process_id, const int iteration, const int all_iterations) {
    char loop_operation[70] = {0};
    assert(sprintf(loop_operation, log_loop_operation_fmt, process_id, iteration, all_iterations) > 0);
    print(loop_operation);
}

void debug_printf(char const *const fmt, ...) {
    if (!DC_DEBUG_PRINT)
        return;

    va_list ptr;
    va_start(ptr, fmt);
    vprintf(fmt, ptr);
    va_end(ptr);
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
