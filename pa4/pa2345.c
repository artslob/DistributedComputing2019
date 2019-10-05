#include <assert.h>

#include "process_common.h"
#include "child.h"
#include "lamport.h"


int request_cs(const void *self) {
    ProcessContext *context = (ProcessContext *) self;
    timestamp_t local_time = lamport_inc_get_time();
    Message request = {.s_header = {
            .s_local_time = local_time, .s_type = CS_REQUEST, .s_magic = MESSAGE_MAGIC, .s_payload_len = 0
    }};
    add_request_to_queue(&context->queue, (Request) {.l_time = local_time, .i = context->id});
    assert(send_multicast((void *) self, &request) == 0);
    return 0;
}

int release_cs(const void *self) {
    ProcessContext *context = (ProcessContext *) self;
    assert(context != NULL);
    return 0;
}
