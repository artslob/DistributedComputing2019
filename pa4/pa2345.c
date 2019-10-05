#include <assert.h>
#include <string.h>

#include "process_common.h"
#include "child.h"
#include "lamport.h"

static int send_to_children(void *self, const Message *msg) {
    ProcessContext *context = (ProcessContext *) self;
    for (local_id i = 1; i < context->N; i++) {
        if (i == context->id) {
            continue;
        }
        if (send(self, i, msg) != 0) {
            return 1;
        }
    }
    return 0;
}


int request_cs(const void *self) {
    ProcessContext *context = (ProcessContext *) self;
    timestamp_t local_time = lamport_inc_get_time();
    Request request = {.l_time = local_time, .i = context->id};
    int s_payload_len = sizeof(request);
    Message request_message = {.s_header = {
            .s_local_time=local_time, .s_type=CS_REQUEST, .s_magic=MESSAGE_MAGIC, .s_payload_len=s_payload_len
    }};
    memcpy(request_message.s_payload, &request, s_payload_len);
    add_request_to_queue(&context->queue, request);
    assert(send_to_children((void *) self, &request_message) == 0);
    return 0;
}

static void remove_own_request_from_queue(RequestQueue *queue) {
    for (int i = 0; i < queue->length - 1; i++) {
        queue->array[i] = queue->array[i + 1];
    }
    queue->length = queue->length - 1;
}

int release_cs(const void *self) {
    ProcessContext *context = (ProcessContext *) self;
    assert(context != NULL);
    timestamp_t local_time = lamport_inc_get_time();
    Message request = {.s_header = {
            .s_local_time = local_time, .s_type = CS_RELEASE, .s_magic = MESSAGE_MAGIC, .s_payload_len = 0
    }};
    remove_own_request_from_queue(&context->queue);
    assert(send_to_children((void *) self, &request) == 0);
    return 0;
}
