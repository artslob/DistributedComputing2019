#ifndef __DC_PROCESS_COMMON__H
#define __DC_PROCESS_COMMON__H

#include <stdio.h>

#include "ipc.h"
#include "pipes.h"

typedef struct {
    timestamp_t l_time;
    local_id i;
} Request;

typedef struct {
    int length;
    Request array[MAX_PROCESS_ID];
} RequestQueue;

typedef struct {
    local_id id;
    local_id N;
    pipe_t **pipes;
    FILE *events_log_fd;
    int mutexl;
    RequestQueue queue;
} ProcessContext;

void receive_all_done(ProcessContext context);

void receive_all_started(ProcessContext context);

#endif //__DC_PROCESS_COMMON__H
