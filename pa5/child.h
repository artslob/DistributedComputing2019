#ifndef __DC_CHILD__H
#define __DC_CHILD__H

#include "process_common.h"


typedef struct {
    local_id from;
} Request;


void child_work(ProcessContext context);

int fork_can_be_requested(const Fork fork);

void init_forks_array(Fork *forks, local_id forks_length, local_id child_id);

#endif // __DC_CHILD__H
