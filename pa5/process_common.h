#ifndef __DC_PROCESS_COMMON__H
#define __DC_PROCESS_COMMON__H

#include <stdio.h>

#include "ipc.h"
#include "pipes.h"


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
    /** Always should be equal to N - 1. */
    const local_id forks_length;
    /**
     * Length always should be equal to N - 1. N - 1 is number of child processes. So process will have fork
     * associated with itself (its index is ProcessContext.id - 1) but such fork should be ignored.
     * For example, if N = 4 (param -p = 3):
     * Parent`s id is 0.
     * Children`s ids are 1, 2 and 3.
     * Child with id = 2 will have such `forks` array with length = 3:
     * [ [0]: fork for 1 process; [1]: ignored; [2]: fork for 3 process; ]
     * index 1 is ignored because its fork for 2 process in array of 2 process.
     */
    Fork forks[MAX_PROCESS_ID];
} ProcessContext;

void receive_all_done(ProcessContext context);

void receive_all_started(ProcessContext context);

#endif //__DC_PROCESS_COMMON__H
