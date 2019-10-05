#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>

#include "pipes.h"
#include "log.h"
#include "process_common.h"
#include "lamport.h"
#include "ipc.h"
#include "pa2345.h"


static void handle_requests(ProcessContext context);

static void useful_work(ProcessContext context);

static void send_started(ProcessContext context);

static void send_done(ProcessContext context);


void child_work(ProcessContext context) {
    close_unused_pipes(context.pipes, context.N, context.id);

    send_started(context);
    log_started(context.events_log_fd, context.id);

    receive_all_started(context);
    log_received_all_started(context.events_log_fd, context.id);

    handle_requests(context);

    close_process_pipes(context.pipes, context.N, context.id);
    fclose(context.events_log_fd);
}


void add_request_to_queue(RequestQueue *queue, Request request) {
    if (queue->length == 0) {
        queue->array[queue->length++] = request;
        return;
    }
    queue->length++;
    assert(queue->length <= MAX_PROCESS_ID);
    for (int i = 0; i < queue->length - 1; i++) {
        Request compared_request = queue->array[i];
        int request_have_smaller_time = request.l_time < compared_request.l_time;
        int time_is_equal_but_smaller_id = request.l_time == compared_request.l_time && request.i < compared_request.i;
        if (request_have_smaller_time || time_is_equal_but_smaller_id) {
            for (int j = queue->length - 2; j >= i; j--) {
                queue->array[j + 1] = queue->array[j];
            }
            queue->array[i] = request;
            return;
        }
    }
    queue->array[queue->length - 1] = request;
}

void remove_first_request_from_queue(RequestQueue *queue) {
    for (int i = 0; i < queue->length - 1; i++) {
        queue->array[i] = queue->array[i + 1];
    }
    queue->length = queue->length - 1;
}

static void handle_requests(ProcessContext context) {
    const int CHILDREN_COUNT = context.N - 2; // minus parent and current process
    int stop_signal_received = 0;
    int done_messages_count = 0;
    int reply_count = 0;

    useful_work(context);
    send_done(context);
    log_done(context.events_log_fd, context.id);

    while (1) {
        if (stop_signal_received && done_messages_count == CHILDREN_COUNT) {
            log_received_all_done(context.events_log_fd, context.id);
            return;
        }

        Message incoming_message;
        assert(receive_any(&context, &incoming_message) == 0);
        assert(incoming_message.s_header.s_magic == MESSAGE_MAGIC);
        lamport_receive_time(incoming_message.s_header.s_local_time);

        if (incoming_message.s_header.s_type == STOP) {
            stop_signal_received++;
            assert(incoming_message.s_header.s_payload_len == 0);
            assert(stop_signal_received == 1); // check stop signal received only once
            continue;
        }

        if (incoming_message.s_header.s_type == DONE) {
            done_messages_count++;
            assert(incoming_message.s_header.s_payload_len == strlen(incoming_message.s_payload) + 1);
            assert(done_messages_count <= CHILDREN_COUNT);
            continue;
        }

        if (incoming_message.s_header.s_type == CS_REPLY) {
            reply_count++;
            assert(incoming_message.s_header.s_payload_len == 0);
            assert(reply_count <= CHILDREN_COUNT);
            // TODO if reply == children and iterations remains and our request is first, go to cs
            continue;
        }

        if (incoming_message.s_header.s_type == CS_REQUEST) {
            Request request;
            assert(incoming_message.s_header.s_payload_len == sizeof(request));
            memcpy(&request, incoming_message.s_payload, incoming_message.s_header.s_payload_len);
            add_request_to_queue(&context.queue, request);
            Message reply = {.s_header = {
                    .s_magic=MESSAGE_MAGIC, .s_local_time=lamport_inc_get_time(), .s_type=CS_REPLY, .s_payload_len=0
            }};
            assert(send(&context, request.i, &reply) == 0);
            continue;
        }

        if (incoming_message.s_header.s_type == CS_RELEASE) {
            assert(incoming_message.s_header.s_payload_len == 0);
            remove_first_request_from_queue(&context.queue);
        }
    }
}

static void useful_work(ProcessContext context) {
    const int LOOP_COUNT = context.id * 5;
    for (int i = 1; i <= LOOP_COUNT; i++) {
        log_loop_operation(context.id, i, LOOP_COUNT);
    }
}

static void send_started(ProcessContext context) {
    Message msg = {.s_header = {
            .s_magic = MESSAGE_MAGIC, .s_type = STARTED, .s_local_time = lamport_inc_get_time()
    }};
    int length = sprintf(msg.s_payload, log_started_fmt, lamport_get_time(), context.id, getpid(), getppid(), 0);
    msg.s_header.s_payload_len = length + 1;
    assert(send_multicast(&context, &msg) == 0);
}

static void send_done(ProcessContext context) {
    Message msg = {.s_header = {
            .s_magic = MESSAGE_MAGIC, .s_type = DONE, .s_local_time = lamport_inc_get_time()
    }};
    int length = sprintf(msg.s_payload, log_done_fmt, lamport_get_time(), context.id, 0);
    msg.s_header.s_payload_len = length + 1;
    assert(send_multicast(&context, &msg) == 0);
}
