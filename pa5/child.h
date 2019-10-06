#ifndef __DC_CHILD__H
#define __DC_CHILD__H

#include "process_common.h"

void add_request_to_queue(RequestQueue *queue, Request request);

void remove_first_request_from_queue(RequestQueue *queue);

void child_work(ProcessContext context);

#endif // __DC_CHILD__H
