#include <string.h>
#include <assert.h>

#include "ipc.h"
#include "main.h"
#include "log.h"
#include "lamport.h"


static void receive_all_with_type(ProcessContext context, MessageType type) {
    for (local_id from = 1; from < context.N; from++) {
        if (from == context.id)
            continue;
        Message msg;
        assert(receive(&context, from, &msg) == 0);
        assert(msg.s_header.s_magic == MESSAGE_MAGIC);
        assert(msg.s_header.s_type == type);
        lamport_receive_time(msg.s_header.s_local_time);
        switch (type) {
            case STARTED:
            case DONE:
                debug_printf("process %d receive msg with length %lu: %s",
                             context.id, strlen(msg.s_payload), msg.s_payload);
                break;
            default:
                debug_printf("process %d receive msg with type %d.\n", context.id, type);
                break;
        }
    }
}

void receive_all_done(ProcessContext context) {
    receive_all_with_type(context, DONE);
}

void receive_all_started(ProcessContext context) {
    receive_all_with_type(context, STARTED);
}

