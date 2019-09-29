#include <string.h>
#include <assert.h>

#include "ipc.h"
#include "main.h"
#include "log.h"
#include "lamport.h"


void receive_all_done(ProcessContext context) {
    for (local_id from = 1; from < context.N; from++) {
        if (from == context.id)
            continue;
        Message msg;
        assert(receive(&context, from, &msg) == 0);
        assert(msg.s_header.s_magic == MESSAGE_MAGIC);
        assert(msg.s_header.s_type == DONE);
        lamport_receive_time(msg.s_header.s_local_time);
        debug_printf("process %d receive msg with length %lu: %s", context.id, strlen(msg.s_payload), msg.s_payload);
    }
}

void receive_all_started(ProcessContext context) {
    for (local_id from = 1; from < context.N; from++) {
        if (from == context.id)
            continue;
        Message msg;
        assert(receive(&context, from, &msg) == 0);
        assert(msg.s_header.s_magic == MESSAGE_MAGIC);
        assert(msg.s_header.s_type == STARTED);
        lamport_receive_time(msg.s_header.s_local_time);
        debug_printf("process %d receive msg with length %lu: %s", context.id, strlen(msg.s_payload), msg.s_payload);
    }
}

