#ifndef __DC_MAIN__H
#define __DC_MAIN__H

#include "ipc.h"
#include "pipes.h"

static const char *const PROCESS_ARG = "-p";

typedef struct {
    local_id id;
    pipe_t **pipes;
    local_id N;
    int events_log_fd;
} ProcessContext;

#endif // __DC_MAIN__H
