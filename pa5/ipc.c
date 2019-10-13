#include <unistd.h>
#include <errno.h>
#include <assert.h>

#include "ipc.h"
#include "process_common.h"
#include "pipes.h"


typedef enum ReadStatus {
    RS_SUCCESS = 0,
    RS_ERROR = 1,
    RS_NO_DATA = 2,
} ReadStatus;

int send(void *self, local_id dst, const Message *msg) {
    ProcessContext *context = (ProcessContext *) self;
    pipe_t pipe = get_pipe(context->pipes, context->id, dst);
    int written = write(pipe.write_fd, msg, sizeof(MessageHeader) + msg->s_header.s_payload_len);
    return (written < 0) ? RS_ERROR : RS_SUCCESS;
}

int send_multicast(void *self, const Message *msg) {
    ProcessContext *context = (ProcessContext *) self;
    for (local_id i = 0; i < context->N; i++) {
        if (i == context->id) {
            continue;
        }
        if (send(self, i, msg) != RS_SUCCESS) {
            return RS_ERROR;
        }
    }
    return RS_SUCCESS;
}

ReadStatus blocking_payload_read(int read_fd, Message *msg) {
    const int payload_len = msg->s_header.s_payload_len;
    assert(payload_len >= 0);
    if (payload_len == 0)
        return RS_SUCCESS;

    while (1) {
        const int count = read(read_fd, msg->s_payload, payload_len);
        if (count < 0 && errno == EAGAIN) {
            continue; // TODO sleep?
        }
        return (count == payload_len) ? RS_SUCCESS : RS_ERROR;
    }
}

ReadStatus asynchronous_header_read(int read_fd, Message *msg) {
    const int header_size = sizeof(msg->s_header);
    const int count = read(read_fd, &msg->s_header, header_size);

    if ((count < 0 && errno == EAGAIN) || count == 0)
        return RS_NO_DATA;

    return (count == header_size) ? RS_SUCCESS : RS_ERROR;
}

int receive(void *self, local_id from, Message *msg) {
    ProcessContext *context = (ProcessContext *) self;
    const int read_fd = get_pipe(context->pipes, from, context->id).read_fd;

    while (1) {
        ReadStatus status = asynchronous_header_read(read_fd, msg);
        if (status == RS_NO_DATA)
            continue;
        if (status == RS_SUCCESS)
            break;
        if (status == RS_ERROR)
            return RS_ERROR;
        assert(0); // should be unreachable
    }

    if (msg->s_header.s_magic != MESSAGE_MAGIC)
        return RS_ERROR;

    return blocking_payload_read(read_fd, msg);
}

int receive_any(void *self, Message *msg) {
    ProcessContext *context = (ProcessContext *) self;
    for (local_id from = 0; from < context->N; from++) {
        if (from == context->id) {
            continue;
        }
        const int read_fd = get_pipe(context->pipes, from, context->id).read_fd;
        ReadStatus status = asynchronous_header_read(read_fd, msg);
        if (status == RS_NO_DATA)
            continue;
        if (status == RS_ERROR)
            return RS_ERROR;
        assert(status == RS_SUCCESS);

        if (msg->s_header.s_magic != MESSAGE_MAGIC)
            return RS_ERROR;

        return blocking_payload_read(read_fd, msg);
    }
    return -1; // when no messages
}
