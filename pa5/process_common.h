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

typedef enum {
    FO_NOT_OWNS = 0,
    FO_OWNS = 1,
} FORK_OWNERSHIP;

typedef enum {
    FS_CLEAN = 0,
    FS_DIRTY = 1,
} FORK_STATE;

typedef enum {
    FR_NOT_REQUESTED = 0,
    FR_REQUESTED = 1,
} FORK_REQUEST;

typedef struct {
    /** Indicates if process have common fork with another process. */
    FORK_OWNERSHIP ownership;
    /** Indicates if process have common fork with another process and this fork is dirty or not. */
    FORK_STATE state;
    /** Indicates if process have request for common fork with another process. */
    FORK_REQUEST request;
} Fork;

typedef struct {
    local_id id;
    local_id N;
    pipe_t **pipes;
    FILE *events_log_fd;
    int mutexl;
    RequestQueue queue;
    /** Always should be equal to N - 1; Defined here only for convenience. */
    const int fork_length;
    Fork forks[];
} ProcessContext;

void receive_all_done(ProcessContext context);

void receive_all_started(ProcessContext context);

#endif //__DC_PROCESS_COMMON__H
