#include <stdio.h>
#include <string.h>

#include "ipc.h"
#include "main.h"

void receive_all_done(ProcessContext context) {
    for (local_id from = 1; from < context.N; from++) {
        if (from == context.id)
            continue;
        Message msg;
        if (receive(&context, from, &msg))
            printf("got error while receiving msg from %d to %d.\n", from, context.id);
        if (msg.s_header.s_magic != MESSAGE_MAGIC)
            printf("got wrong magic in message while starting.\n");
        if (msg.s_header.s_type != DONE)
            printf("got wrong type of message while starting.\n");
        printf("process %d receive msg with length %lu: %s\n", context.id, strlen(msg.s_payload), msg.s_payload);
    }
}

void receive_all_started(ProcessContext context) {
    for (local_id from = 1; from < context.N; from++) {
        if (from == context.id)
            continue;
        Message msg;
        if (receive(&context, from, &msg))
            printf("got error while receiving msg from %d to %d.\n", from, context.id);
        if (msg.s_header.s_magic != MESSAGE_MAGIC)
            printf("got wrong magic in message while starting.\n");
        if (msg.s_header.s_type != STARTED)
            printf("got wrong type of message while starting.\n");
        printf("process %d receive msg with length %lu: %s\n", context.id, strlen(msg.s_payload), msg.s_payload);
    }
}

