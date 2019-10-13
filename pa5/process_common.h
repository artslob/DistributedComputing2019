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
    /** Always should be equal to N. */
    const local_id forks_length;
    /** Length always should be equal to N. So process will have fork associated with itself
     * (with index = id of process) but such fork should be ignored.
     * For example, if N = 4 (param -p = 3): Parent`s id is 0. Children`s ids are 1, 2 and 3.
     * Child with id = 2 will have such `forks` array with length = 4:
     * [ [0]: ignored; [1]: fork for 1 process; [2]: ignored; [3]: fork for 3 process; ]
     * index 0 is ignored because its fork with parent process.
     * index 2 is ignored because its fork with process itself.
     */
    Fork forks[MAX_PROCESS_ID];
} ProcessContext;

void receive_all_done(ProcessContext context);

void receive_all_started(ProcessContext context);

#endif //__DC_PROCESS_COMMON__H
