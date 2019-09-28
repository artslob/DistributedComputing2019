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

balance_t get_child_balance(local_id child_id, char *argv[]);


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
        if (pid > 0) {
            debug_printf("I am parent\n");
            continue;
        }
        // its child when pid = 0
        debug_printf("I am child with id %d\n", child_id);
        balance_t child_balance = get_child_balance(child_id, argv);
        ProcessContext context = {
                .id = child_id,
                .pipes = pipes,
                .N = N,
                .events_log_fd = events_log_file,
                .balance = child_balance,
        };
        child_work(context);
        exit(0);
    }

    ProcessContext context = {.id = PARENT_ID, .pipes = pipes, .N = N, .events_log_fd = events_log_file, .balance = -1};

    close_unused_pipes(context.pipes, context.N, context.id);
    log_started(context.events_log_fd, context.id);

    receive_all_started(context);
    log_received_all_started(context.events_log_fd, context.id);

    bank_robbery(&context, N - 1);

    log_done(context.events_log_fd, context.id);

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
// Index in argv from which balance numbers are provided.
static const int BALANCE_ARGC_START = 3;

local_id parse_cli_args(int argc, char *argv[]) {
    // argv[0] = name of program.
    // argv[1] = -p
    // argv[2] = X - number of child processes.
    int expected_minimal_argc = BALANCE_ARGC_START;
    if (argc < expected_minimal_argc) {
        fatalf("not enough arguments.\n");
    }
    if (strcmp(argv[1], PROCESS_ARG) != 0) {
        fatalf("wrong 'process argument' flag.\n");
    }
    local_id X = strtol(argv[2], NULL, 10);
    if (X < 1 || 9 < X) {
        fatalf("process argument is out of range.\n");
    }
    // argv[3] = balance of 1 child process.
    // ...
    // argv[2 + X] = balance of X child process.
    int expected_minimal_argc_with_balances = expected_minimal_argc + X;
    if (argc < expected_minimal_argc_with_balances) {
        int got_count_balances = argc - expected_minimal_argc;
        fatalf("balance is not provided for each process. expected %d numbers, got %d.\n", X, got_count_balances);
    }
    return X + 1;
}

/** Parses cli args and returns balance of child process.
 * Method assumes that correct number of cli args is provided.
 * @param child_id: child process id which balance is returned.
 * @returns: balance number that lies in interval [1; 99].
 * */
balance_t get_child_balance(local_id child_id, char *argv[]) {
    balance_t balance = strtol(argv[BALANCE_ARGC_START + child_id - 1], NULL, 10);
    if (balance < 1 || 99 < balance) {
        fatalf("balance for child %d not in interval [1; 99], got number: %d.\n", child_id, balance);
    }
    return balance;
}
