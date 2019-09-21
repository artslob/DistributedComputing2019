#include <unistd.h>

#include "ipc.h"
#include "main.h"
#include "pipes.h"


int send(void * self, local_id dst, const Message * msg) {
    ProcessContext* context = (ProcessContext*) self;
    pipe_t pipe = get_pipe(context->pipes, context->id, dst);
    int written = write(pipe.write_fd, msg, sizeof(MessageHeader) + msg->s_header.s_payload_len);
    return (written < 0) ? 1 : 0;
}

int send_multicast(void * self, const Message * msg) {
    ProcessContext* context = (ProcessContext*) self;
    for (local_id i = 0; i < context->N; i++) {
        if (i == context->id) {
            continue;
        }
        if (send(self, i, msg)) {
            return 1;
        }
    }
    return 0;
}

int receive(void * self, local_id from, Message * msg) {
    ProcessContext* context = (ProcessContext*) self;
    int read_fd = get_pipe(context->pipes, from, context->id).read_fd;

    MessageHeader header;
    if (read(read_fd, &header, sizeof(header)) < 0)
        return 1;

    if (header.s_magic != MESSAGE_MAGIC)
        return 1;

    msg->s_header = header;

    if (read(read_fd, msg->s_payload, header.s_payload_len) < 0)
        return 1;

    return 0;
}
