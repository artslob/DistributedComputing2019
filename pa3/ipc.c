#include <unistd.h>
#include <errno.h>

#include "ipc.h"
#include "main.h"
#include "pipes.h"


int send(void *self, local_id dst, const Message *msg) {
    ProcessContext *context = (ProcessContext *) self;
    pipe_t pipe = get_pipe(context->pipes, context->id, dst);
    int written = write(pipe.write_fd, msg, sizeof(MessageHeader) + msg->s_header.s_payload_len);
    return (written < 0) ? 1 : 0;
}

int send_multicast(void *self, const Message *msg) {
    ProcessContext *context = (ProcessContext *) self;
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

int receive(void *self, local_id from, Message *msg) {
    ProcessContext *context = (ProcessContext *) self;
    const int read_fd = get_pipe(context->pipes, from, context->id).read_fd;
    const int header_size = sizeof(msg->s_header);

    while (1) {
        const int count = read(read_fd, &msg->s_header, header_size);
        if (count < 0 && errno == EAGAIN) {
            // TODO sleep?
            continue;
        }
        if (count <= 0 || count != header_size) {
            return 1;
        }
        break;
    }

    if (msg->s_header.s_magic != MESSAGE_MAGIC)
        return 1;

    const int payload_len = msg->s_header.s_payload_len;
    if (payload_len == 0) {
        return 0;
    }

    while (1) {
        const int count = read(read_fd, msg->s_payload, payload_len);
        if (count < 0 && errno == EAGAIN) {
            // TODO sleep?
            continue;
        }
        if (count <= 0 || count != payload_len) {
            return 1;
        }
        return 0;
    }
}

int receive_any(void *self, Message *msg) {
    ProcessContext *context = (ProcessContext *) self;
    while (1) {
        for (local_id from = 0; from < context->N; from++) {
            const int read_fd = get_pipe(context->pipes, from, context->id).read_fd;
            const int header_size = sizeof(msg->s_header);

            int count = read(read_fd, &msg->s_header, header_size);
            if (count < 0 && errno == EAGAIN) {
                // TODO sleep?
                continue;
            }
            if (count <= 0 || count != header_size) {
                return 1;
            }
            if (msg->s_header.s_magic != MESSAGE_MAGIC) {
                return 1;
            }
            const int payload_len = msg->s_header.s_payload_len;
            if (payload_len == 0) {
                return 0;
            }
            while (1) {
                count = read(read_fd, msg->s_payload, payload_len);
                if (count < 0 && errno == EAGAIN) {
                    // TODO sleep?
                    continue;
                }
                if (count <= 0 || count != payload_len) {
                    return 1;
                }
                return 0;
            }
        }
    }
}
