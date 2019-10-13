#include <assert.h>
#include <string.h>

#include "process_common.h"
#include "child.h"
#include "lamport.h"


int request_cs(const void *self) {
    ProcessContext *context = (ProcessContext *) self;
    timestamp_t local_time = lamport_inc_get_time();
    Request request = {.from = context->id};
    int s_payload_len = sizeof(request);
    Message request_message = {.s_header = {
            .s_local_time=local_time, .s_type=CS_REQUEST, .s_magic=MESSAGE_MAGIC, .s_payload_len=s_payload_len
    }};
    memcpy(request_message.s_payload, &request, s_payload_len);

    for (local_id i = 1; i < context->N; i++) {
        if (i == context->id)
            continue;
        Fork *current_fork = &context->forks[i];
        if (fork_can_be_requested(*current_fork)) {
            current_fork->request = FR_MISSING_TOKEN;
            assert(send(context, i, &request_message) == 0);
        }
    }
    return 0;
}

int release_cs(const void *self) {
    ProcessContext *context = (ProcessContext *) self;
    timestamp_t local_time = lamport_inc_get_time();
    Request request = {.from = context->id};
    int s_payload_len = sizeof(request);
    Message release_message = {.s_header = {
            .s_local_time = local_time, .s_type = CS_RELEASE, .s_magic = MESSAGE_MAGIC, .s_payload_len = s_payload_len
    }};
    memcpy(release_message.s_payload, &request, s_payload_len);

    for (local_id i = 1; i < context->forks_length; i++) {
        if (i == context->id)
            continue;
        Fork *current_fork = &context->forks[i];
        if (current_fork->ownership == FO_OWNS && current_fork->state == FS_DIRTY
            && current_fork->request == FR_TOKEN) {
            current_fork->ownership = FO_NOT_OWNS;
            assert(send(context, i, &release_message) == 0);
        }
    }

    return 0;
}
