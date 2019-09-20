#ifndef __DC_PIPES__H
#define __DC_PIPES__H

#include "ipc.h"

typedef struct {
    int read_fd;
    int write_fd;
} pipe_t;


pipe_t** create_pipes(int N, int pipes_log_fd);
void close_unused_pipes(pipe_t** pipes, int N, local_id process_id);
void close_process_pipes(pipe_t** pipes, int N, local_id process_id);
pipe_t get_pipe(pipe_t** pipes, local_id from, local_id to);

#endif // __DC_PIPES__H
