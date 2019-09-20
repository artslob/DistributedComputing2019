#ifndef __DC_PIPES__H
#define __DC_PIPES__H

typedef struct {
    int read_fd;
    int write_fd;
} pipe_t;


pipe_t** create_pipes(int N, int pipes_log_fd);

#endif // __DC_PIPES__H
