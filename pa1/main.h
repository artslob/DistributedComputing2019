#ifndef __DC_MAIN__H
#define __DC_MAIN__H

static const char * const PROCESS_ARG = "-p";

typedef struct {
    int read_fd;
    int write_fd;
} pipe_t;

#endif // __DC_MAIN__H
