#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/types.h>
#include <assert.h>

#include "ipc.h"
#include "common.h"
#include "log.h"
#include "pipes.h"
#include "child.h"
#include "process_common.h"
#include "lamport.h"


void wait_children();

static void send_stop_signal_to_children(ProcessContext context);

static int get_mutexl_param(int argc, char **argv);

static local_id get_N_param(int argc, char **argv);


/**
 * Order of args can be (but not required be exactly) like this:
 * argv[0] = name of program (auto provided).
 * argv[1] = -p
 * argv[2] = X - number of child processes.
 * argv[3] = --mutexl - optional flag to enable "critical section".
 */
int main(int argc, char *argv[]) {
    const local_id N = get_N_param(argc, argv);
    const int mutexl = get_mutexl_param(argc, argv);
    debug_printf("N is %d\n", N);
    debug_printf("mutexl is %d\n", mutexl);

    FILE *pipes_log_file = fopen(pipes_log, "w+t");
    FILE *events_log_file = fopen(events_log, "w+t");

    pipe_t **pipes = create_pipes(N, pipes_log_file);
    fclose(pipes_log_file);
    make_pipes_asynchronous(pipes, N);

    for (local_id child_id = 1; child_id < N; child_id++) {
        pid_t pid = fork();
        if (pid < 0) {
            fatalf("error on fork() call!");
        }
        if (pid > 0) {
            debug_printf("I am parent\n");
            continue;
        }
        // its child when pid = 0
        debug_printf("I am child with id %d\n", child_id);
        ProcessContext context = {
                .id = child_id,
                .pipes = pipes,
                .N = N,
                .events_log_fd = events_log_file,
                .mutexl = mutexl,
                .forks_length = N,
                .forks = {{0}}
        };
        init_forks_array(context.forks, N, child_id);
        child_work(context);
        exit(0);
    }

    ProcessContext context = {.id=PARENT_ID, .pipes=pipes, .N=N, .events_log_fd=events_log_file, .mutexl=mutexl};

    close_unused_pipes(context.pipes, context.N, context.id);

    log_started(context.events_log_fd, context.id);
    receive_all_started(context);
    log_received_all_started(context.events_log_fd, context.id);

    log_done(context.events_log_fd, context.id);
    receive_all_done(context);
    log_received_all_done(context.events_log_fd, context.id);

    send_stop_signal_to_children(context);

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

static void send_stop_signal_to_children(ProcessContext context) {
    timestamp_t timestamp = lamport_inc_get_time();
    Message msg = {.s_header = {
            .s_magic = MESSAGE_MAGIC, .s_payload_len = 0, .s_type = STOP, .s_local_time = timestamp
    }};
    assert(send_multicast(&context, &msg) == 0);
}

static const char *const MUTEXL_ARG = "--mutexl";

/**
 * Parses optional "--mutexl" cli parameter.
 * @returns: 1 if provided, else 0.
 */
static int get_mutexl_param(int argc, char **argv) {
    // start from 1 because 0 param is name of program
    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], MUTEXL_ARG) == 0) {
            return 1;
        }
    }
    return 0;
}

static const char *const PROCESS_ARG = "-p";

/**
 * Parses required "-p" cli parameter - number of child processes = X.
 * @returns N: number of all processes = X + 1 (number of children + 1 parent).
 */
static local_id get_N_param(int argc, char **argv) {
    const int INVALID_INDEX = -1;
    int process_flag_index = INVALID_INDEX;

    // start from 1 because 0 param is name of program
    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], PROCESS_ARG) == 0) {
            process_flag_index = i;
            break;
        }
    }
    if (process_flag_index == INVALID_INDEX) {
        fatalf("Error: Please, provide \"%s\" flag to specify X - number of child processes.\n", PROCESS_ARG);
    }
    const int process_arg_index = process_flag_index + 1;
    if (argc <= process_arg_index) {
        fatalf("Error: After \"%s\" flag specify integer X - number of child processes.\n", PROCESS_ARG);
    }
    local_id X = strtol(argv[process_arg_index], NULL, 10);
    if (X < 1 || 9 < X) {
        fatalf("Error: Process argument X is out of range, should be in [1; 9].\n");
    }
    return X + 1;
}
