#ifndef __DC_MAIN__H
#define __DC_MAIN__H

#include <stdio.h>

#include "ipc.h"
#include "pipes.h"

typedef struct {
    local_id id;
    local_id N;
    pipe_t **pipes;
    FILE *events_log_fd;
} ProcessContext;

#endif // __DC_MAIN__H
