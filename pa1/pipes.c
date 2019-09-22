#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "pipes.h"
#include "log.h"
#include "ipc.h"


pipe_t get_pipe(pipe_t **pipes, local_id from, local_id to) {
    return pipes[from][to];
}

void close_unused_pipes(pipe_t **pipes, local_id N, local_id process_id) {
    local_id rows_count = N, columns_count = N;

    for (local_id i = 0; i < rows_count - 1; i++) {
        for (local_id j = i + 1; j < columns_count; j++) {
            if (i == process_id) {
                close(pipes[process_id][j].read_fd);
                close(pipes[j][process_id].write_fd);
                continue;
            }
            if (j == process_id) {
                close(pipes[process_id][i].read_fd);
                close(pipes[i][process_id].write_fd);
                continue;
            }
            close(pipes[i][j].read_fd);
            close(pipes[i][j].write_fd);
            close(pipes[j][i].read_fd);
            close(pipes[j][i].write_fd);
        }
    }
}

void close_process_pipes(pipe_t **pipes, local_id N, local_id process_id) {
    for (local_id i = 0; i < N; i++) {
        if (i == process_id) {
            continue;
        }
        close(pipes[process_id][i].write_fd);
        close(pipes[i][process_id].read_fd);
    }
}


pipe_t **create_pipes(local_id N, FILE* pipes_log_file) {
    local_id rows_count = N, columns_count = N;
    int length = sizeof(pipe_t *) * rows_count + sizeof(pipe_t) * rows_count * columns_count;
    pipe_t **array = (pipe_t **) malloc(length);

    // this pointer contains address of first element in array
    pipe_t *first_array_element_address = (pipe_t *) (array + rows_count);

    for (int i = 0; i < rows_count; i++) {
        array[i] = first_array_element_address + columns_count * i;
    }

    int pipefd[2];

    for (local_id i = 0; i < rows_count - 1; i++) {
        for (local_id j = i + 1; j < columns_count; j++) {
            if (pipe(pipefd) != 0) {
                fatalf("error occured while creating pipe.\n");
            }
            array[i][j].read_fd = pipefd[0];
            array[i][j].write_fd = pipefd[1];
            printf("%d %d: %2d %2d\n", i, j, array[i][j].read_fd, array[i][j].write_fd);
            log_pipe_created(pipes_log_file, i, j, pipefd[0], pipefd[1]);

            if (pipe(pipefd) != 0) {
                fatalf("error occured while creating pipe.\n");
            }
            array[j][i].read_fd = pipefd[0];
            array[j][i].write_fd = pipefd[1];
            printf("%d %d: %2d %2d\n", j, i, array[j][i].read_fd, array[j][i].write_fd);
            log_pipe_created(pipes_log_file, j, i, pipefd[0], pipefd[1]);
        }
    }

    return array;
}
