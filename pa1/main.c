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


int parse_cli_args(int argc, char* argv[]);

int main(int argc, char* argv[])
{
    int N = parse_cli_args(argc, argv);
    printf("N is %d\n", N);

    int pipes_log_fd = open(pipes_log, O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR);

    pipe_t** pipes = create_pipes(N, pipes_log_fd);
    close(pipes_log_fd);

    for (local_id child_id = 1; child_id < N; child_id++) {
        pid_t pid = fork();
        if (pid < 0) {
            fatalf("error on fork() call!");
        }
        if (pid == 0) {
            printf("I am child with id %d\n", child_id);
            exit(0);
        } else {
            printf("I am parent\n");
        }
    }

    close_unused_pipes(pipes, N, 0);

    pid_t child_pid = 0;
    int status = 0;
    while ((child_pid = wait(&status)) > 0) {
        printf("child process %d finished with %d.\n", child_pid, status);
    }

    return 0;
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
