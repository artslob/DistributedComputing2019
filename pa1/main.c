#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "main.h"
#include "ipc.h"
#include "common.h"
#include "log.h"
#include "pipes.h"
#include "pa1.h"


void child_work(ProcessContext context);
void receive_all_started(ProcessContext context);
void receive_all_done(ProcessContext context);
int parse_cli_args(int argc, char* argv[]);

int main(int argc, char* argv[])
{
    int N = parse_cli_args(argc, argv);
    printf("N is %d\n", N);

    int pipes_log_fd = open(pipes_log, O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR);
    int events_log_fd = open(events_log, O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR);

    pipe_t** pipes = create_pipes(N, pipes_log_fd);
    close(pipes_log_fd);

    for (local_id child_id = 1; child_id < N; child_id++) {
        pid_t pid = fork();
        if (pid < 0) {
            fatalf("error on fork() call!");
        }
        if (pid == 0) {
            printf("I am child with id %d\n", child_id);
            ProcessContext context = {.id = child_id, .pipes = pipes, .N = N, .events_log_fd = events_log_fd};
            child_work(context);
            exit(0);
        } else {
            printf("I am parent\n");
        }
    }

    ProcessContext context = {.id = PARENT_ID, .pipes = pipes, .N = N, .events_log_fd = events_log_fd};
    close_unused_pipes(context.pipes, context.N, context.id);
    receive_all_started(context);
    receive_all_done(context);

    pid_t child_pid = 0;
    int status = 0;
    while ((child_pid = wait(&status)) > 0) {
        printf("child process %d finished with %d.\n", child_pid, status);
    }

    close_process_pipes(context.pipes, context.N, context.id);
    close(context.events_log_fd);

    return 0;
}

void child_work(ProcessContext context) {
    close_unused_pipes(context.pipes, context.N, context.id);

    MessageHeader header = {.s_magic = MESSAGE_MAGIC, .s_type = STARTED, .s_local_time = 0};
    Message msg = {.s_header = header};
    int length = sprintf(msg.s_payload, log_started_fmt, context.id, getpid(), getppid());
    msg.s_header.s_payload_len = length + 1;
    if (send_multicast(&context, &msg)) {
        printf("could not send_multicast msg.\n");
    }
    receive_all_started(context);

    header = (MessageHeader) {.s_magic = MESSAGE_MAGIC, .s_type = DONE, .s_local_time = 0};
    msg = (Message) {.s_header = header};
    length = sprintf(msg.s_payload, log_done_fmt, context.id);
    msg.s_header.s_payload_len = length + 1;
    if (send_multicast(&context, &msg)) {
        printf("could not send_multicast msg.\n");
    }
    receive_all_done(context);

    close_process_pipes(context.pipes, context.N, context.id);
    close(context.events_log_fd);
}

void receive_all_done(ProcessContext context) {
    for (local_id from = 1; from < context.N; from++) {
        if (from == context.id)
            continue;
        Message msg;
        if (receive(&context, from, &msg))
            printf("got error while receiving msg from %d to %d.\n", from, context.id);
        if (msg.s_header.s_magic != MESSAGE_MAGIC)
            printf("got wrong magic in message while starting.\n");
        if (msg.s_header.s_type != DONE)
            printf("got wrong type of message while starting.\n");
        printf("process %d receive msg with length %lu: %s\n", context.id, strlen(msg.s_payload), msg.s_payload);
    }
}

void receive_all_started(ProcessContext context) {
    for (local_id from = 1; from < context.N; from++) {
        if (from == context.id)
            continue;
        Message msg;
        if (receive(&context, from, &msg))
            printf("got error while receiving msg from %d to %d.\n", from, context.id);
        if (msg.s_header.s_magic != MESSAGE_MAGIC)
            printf("got wrong magic in message while starting.\n");
        if (msg.s_header.s_type != STARTED)
            printf("got wrong type of message while starting.\n");
        printf("process %d receive msg with length %lu: %s\n", context.id, strlen(msg.s_payload), msg.s_payload);
    }
}

int parse_cli_args(int argc, char* argv[])
{
    if (argc < 3) {
        fatalf("not enough arguments.\n");
    }
    if (strcmp(argv[1], PROCESS_ARG)) {
        fatalf("wrong 'process argument' flag.\n");
    }
    int X = atoi(argv[2]);
    if (1 <= X && X <= 9) {
        return X + 1;
    }
    fatalf("process argument is out of range.\n");
    exit(1);
}
