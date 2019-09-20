#include <unistd.h>

#include "ipc.h"
#include "main.h"
#include "pipes.h"


int send(void * self, local_id dst, const Message * msg) {
    ProcessContext* context = (ProcessContext*) self;
    pipe_t pipe = get_pipe(context->pipes, context->id, dst);
    int status = write(pipe.write_fd, msg, sizeof(MessageHeader) + msg->s_header.s_payload_len);
    return (status > 0) ? 0 : 1;
}

int send_multicast(void * self, const Message * msg) {
    ProcessContext* context = (ProcessContext*) self;
    for (local_id i = 0; i < context->N; i++) {
        if (i == context->id) {
            continue;
        }
        int status = send(self, i, msg);
        if (status != 0) {
            return 1;
        }
    }
    return 0;
}

int receive(void * self, local_id from, Message * msg) {
    return 0;
}
