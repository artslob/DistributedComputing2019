#ifndef __DC_PIPES__H
#define __DC_PIPES__H

#include "ipc.h"

typedef struct {
    int read_fd;
    int write_fd;
} pipe_t;


pipe_t** create_pipes(int N, int pipes_log_fd);
void close_unused_pipes(pipe_t** pipes, int N, local_id process_id);

#endif // __DC_PIPES__H
