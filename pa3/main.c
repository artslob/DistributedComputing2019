#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/types.h>

#include "main.h"
#include "ipc.h"
#include "common.h"
#include "log.h"
#include "pipes.h"
#include "child.h"
#include "process_common.h"


void wait_children();

local_id parse_cli_args(int argc, char *argv[]);


int main(int argc, char *argv[]) {
    local_id N = parse_cli_args(argc, argv);
    debug_printf("N is %d\n", N);

    FILE *pipes_log_file = fopen(pipes_log, "w+t");
    FILE *events_log_file = fopen(events_log, "w+t");

    pipe_t **pipes = create_pipes(N, pipes_log_file);
    fclose(pipes_log_file);

    for (local_id child_id = 1; child_id < N; child_id++) {
        pid_t pid = fork();
        if (pid < 0) {
            fatalf("error on fork() call!");
        }
        if (pid == 0) {
            debug_printf("I am child with id %d\n", child_id);
            ProcessContext context = {.id = child_id, .pipes = pipes, .N = N, .events_log_fd = events_log_file};
            child_work(context);
            exit(0);
        } else {
            debug_printf("I am parent\n");
        }
    }

    ProcessContext context = {.id = PARENT_ID, .pipes = pipes, .N = N, .events_log_fd = events_log_file};

    close_unused_pipes(context.pipes, context.N, context.id);
    log_started(context.events_log_fd, context.id);

    receive_all_started(context);
    log_received_all_started(context.events_log_fd, context.id);

    log_done(context.events_log_fd, context.id);

    //bank_robbery(parent_data);
    //print_history(all);

    receive_all_done(context);
    log_received_all_done(context.events_log_fd, context.id);

    wait_children();

    close_process_pipes(context.pipes, context.N, context.id);
    fclose(context.events_log_fd);

    return 0;
}

void wait_children() {
    pid_t child_pid = 0;
    int status = 0;
    while ((child_pid = wait(&status)) > 0) {
        debug_printf("child process %d finished with %d.\n", child_pid, status);
    }
}

static const char *const PROCESS_ARG = "-p";

local_id parse_cli_args(int argc, char *argv[]) {
    if (argc < 3) {
        fatalf("not enough arguments.\n");
    }
    if (strcmp(argv[1], PROCESS_ARG) != 0) {
        fatalf("wrong 'process argument' flag.\n");
    }
    local_id X = atoi(argv[2]);
    if (1 <= X && X <= 9) {
        return X + 1;
    }
    fatalf("process argument is out of range.\n");
    exit(1);
}
