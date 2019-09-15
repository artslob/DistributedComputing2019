#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "main.h"


int parse_cli_args(int argc, char* argv[]);


int main(int argc, char* argv[])
{
    int N = parse_cli_args(argc, argv);
    printf("N is %d\n", N);

    for (int child_id = 1; child_id < N; child_id++) {
        pid_t pid = fork();
        if (pid < 0) {
            printf("error on fork() call!");
            exit(1);
        }
        if (pid == 0) {
            printf("I am child with id %d\n", child_id);
            exit(0);
        } else {
            printf("I am parent\n");
        }
    }

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
        printf("not enough arguments.\n");
        exit(0);
    }
    if (strcmp(argv[1], PROCESS_ARG)) {
        printf("wrong 'process argument' flag.\n");
        exit(0);
    }
    int X = atoi(argv[2]);
    if (1 <= X && X <= 9) {
        return X + 1;
    }
    printf("process argument is out of range.\n");
    exit(0);
}
