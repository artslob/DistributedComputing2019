#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "main.h"
#include "ipc.h"


int parse_cli_args(int argc, char* argv[]);

pipe_t** create_pipes(int N) {
    int rows_count = N, columns_count = N;
    int length = sizeof(pipe_t*) * rows_count + sizeof(pipe_t) * rows_count * columns_count;
    pipe_t** array = (pipe_t**) malloc(length);

    // this pointer contains address of first element in array
    pipe_t* first_array_element_address = (pipe_t*) (array + rows_count);

    for (int i = 0; i < rows_count; i++) {
        array[i] = first_array_element_address + columns_count * i;
    }

    int pipefd[2];

    for (int i = 0; i < rows_count - 1; i++) {
        for (int j = i + 1; j < columns_count; j++) {
            // TODO: log pipe creation
            if (pipe(pipefd) != 0) {
                printf("error occured while creating pipe.\n");
                exit(1);
            }
            array[i][j].read_fd = pipefd[0];
            array[i][j].write_fd = pipefd[1];
            printf("%d %d: %2d %2d\n", i, j, array[i][j].read_fd, array[i][j].write_fd);

            if (pipe(pipefd) != 0) {
                printf("error occured while creating pipe.\n");
                exit(1);
            }
            array[j][i].read_fd = pipefd[0];
            array[j][i].write_fd = pipefd[1];
            printf("%d %d: %2d %2d\n", j, i, array[j][i].read_fd, array[j][i].write_fd);
        }
    }

    return array;
}


int main(int argc, char* argv[])
{
    int N = parse_cli_args(argc, argv);
    printf("N is %d\n", N);

    pipe_t** pipes = create_pipes(N);
    pipes = NULL;

    for (local_id child_id = 1; child_id < N; child_id++) {
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
