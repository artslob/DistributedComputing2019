#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "pipes.h"
#include "log.h"


pipe_t** create_pipes(int N, int pipes_log_fd) {
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
            if (pipe(pipefd) != 0) {
                fatalf("error occured while creating pipe.\n");
            }
            array[i][j].read_fd = pipefd[0];
            array[i][j].write_fd = pipefd[1];
            printf("%d %d: %2d %2d\n", i, j, array[i][j].read_fd, array[i][j].write_fd);
            log_pipe_created(pipes_log_fd, i, j, pipefd[0], pipefd[1]);

            if (pipe(pipefd) != 0) {
                fatalf("error occured while creating pipe.\n");
            }
            array[j][i].read_fd = pipefd[0];
            array[j][i].write_fd = pipefd[1];
            printf("%d %d: %2d %2d\n", j, i, array[j][i].read_fd, array[j][i].write_fd);
            log_pipe_created(pipes_log_fd, j, i, pipefd[0], pipefd[1]);
        }
    }

    return array;
}
