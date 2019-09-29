#include <unistd.h>
#include <errno.h>
#include <assert.h>

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

int blocking_payload_read(int read_fd, Message *msg) {
    const int payload_len = msg->s_header.s_payload_len;
    assert(payload_len >= 0);
    if (payload_len == 0)
        return 0;

    while (1) {
        const int count = read(read_fd, msg->s_payload, payload_len);
        if (count < 0 && errno == EAGAIN) {
            continue; // TODO sleep?
        }
        if (count <= 0 || count != payload_len)
            return 1;
        return 0;
    }
}

int asynchronous_header_read(int read_fd, Message *msg) {
    const int header_size = sizeof(msg->s_header);

    int count = read(read_fd, &msg->s_header, header_size);
    if (count < 0 && errno == EAGAIN) {
        return -1;
    }
    if (count <= 0 || count != header_size) {
        return 1;
    }
    return 0;
}

int receive(void *self, local_id from, Message *msg) {
    ProcessContext *context = (ProcessContext *) self;
    const int read_fd = get_pipe(context->pipes, from, context->id).read_fd;

    while (1) {
        int async_status = asynchronous_header_read(read_fd, msg);
        if (async_status == -1)
            continue;
        if (async_status == 0)
            break;
        if (async_status == 1)
            return 1;
        assert(0);
    }

    if (msg->s_header.s_magic != MESSAGE_MAGIC)
        return 1;

    return blocking_payload_read(read_fd, msg);
}

int receive_any(void *self, Message *msg) {
    ProcessContext *context = (ProcessContext *) self;
    while (1) {
        for (local_id from = 0; from < context->N; from++) {
            const int read_fd = get_pipe(context->pipes, from, context->id).read_fd;
            int async_status = asynchronous_header_read(read_fd, msg);
            if (async_status == -1)
                continue;
            if (async_status == 1)
                return 1;
            assert(async_status == 0);

            if (msg->s_header.s_magic != MESSAGE_MAGIC) {
                return 1;
            }
            return blocking_payload_read(read_fd, msg);
        }
    }
}
