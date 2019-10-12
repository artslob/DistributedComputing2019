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

static void handle_requests_without_cs(ProcessContext context);

static void send_started(ProcessContext context);

static void send_done(ProcessContext context);


void child_work(ProcessContext context) {
    close_unused_pipes(context.pipes, context.N, context.id);

    send_started(context);
    log_started(context.events_log_fd, context.id);

    receive_all_started(context);
    log_received_all_started(context.events_log_fd, context.id);

    if (context.mutexl) {
        handle_requests(context);
    } else {
        handle_requests_without_cs(context);
    }

    close_process_pipes(context.pipes, context.N, context.id);
    fclose(context.events_log_fd);
}

static void handle_requests(ProcessContext context) {
    const int CHILDREN_COUNT = context.N - 2; // minus parent and current process
    const int MAX_LOOP_COUNT = context.id * 5;
    int stop_signal_received = 0;
    int done_messages_count = 0;
    int reply_count = 0;
    int loop_iteration = 1;
    int request_sent = 0;
    int done_sent = 0;

    while (1) {
        if (stop_signal_received && done_messages_count == CHILDREN_COUNT) {
            log_received_all_done(context.events_log_fd, context.id);
            return;
        }

        if (done_sent == 0 && loop_iteration > MAX_LOOP_COUNT) {
            send_done(context);
            log_done(context.events_log_fd, context.id);
            done_sent = 1;
            request_sent = 0;
            continue;
        }

        if (loop_iteration <= MAX_LOOP_COUNT && request_sent == 0) {
            request_cs(&context);
            request_sent = 1;
            continue;
        }

//        if (request_sent && context.queue.array[0].i == context.id && reply_count == CHILDREN_COUNT) {
        if (request_sent && reply_count == CHILDREN_COUNT) {
            log_loop_operation(context.id, loop_iteration++, MAX_LOOP_COUNT);
            release_cs(&context);
            request_sent = 0;
            reply_count = 0;
            continue;
        }

        Message incoming_message;
        assert(receive_any(&context, &incoming_message) == 0);
        assert(incoming_message.s_header.s_magic == MESSAGE_MAGIC);
        lamport_receive_time(incoming_message.s_header.s_local_time);

        switch (incoming_message.s_header.s_type) {
            case STOP: {
                stop_signal_received++;
                assert(incoming_message.s_header.s_payload_len == 0);
                assert(stop_signal_received == 1); // check stop signal received only once
                break;
            }
            case DONE: {
                done_messages_count++;
                assert(incoming_message.s_header.s_payload_len == strlen(incoming_message.s_payload) + 1);
                assert(done_messages_count <= CHILDREN_COUNT);
                break;
            }
            case CS_REPLY: {
                reply_count++;
                assert(request_sent);
//                assert(context.queue.length > 0);
                assert(incoming_message.s_header.s_payload_len == 0);
                assert(reply_count <= CHILDREN_COUNT);
                break;
            }
            case CS_REQUEST: {
                assert(incoming_message.s_header.s_payload_len == 0);
                Message reply = {.s_header = {
                        .s_magic=MESSAGE_MAGIC, .s_local_time=lamport_inc_get_time(), .s_type=CS_REPLY, .s_payload_len=0
                }};
                assert(sizeof(reply) > 0);
//                assert(send(&context, request.i, &reply) == 0);
                break;
            }
            case CS_RELEASE: {
                assert(incoming_message.s_header.s_payload_len == 0);
//                assert(context.queue.array[0].i != context.id);
                break;
            }
            default: {
                assert(0); // unreachable code
                break;
            }
        }
    }
}

static void handle_requests_without_cs(ProcessContext context) {
    const int CHILDREN_COUNT = context.N - 2; // minus parent and current process
    const int MAX_LOOP_COUNT = context.id * 5;
    int stop_signal_received = 0;
    int done_messages_count = 0;

    for (int i = 1; i <= MAX_LOOP_COUNT; i++) {
        log_loop_operation(context.id, i, MAX_LOOP_COUNT);
    }
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

        switch (incoming_message.s_header.s_type) {
            case DONE: {
                done_messages_count++;
                assert(incoming_message.s_header.s_payload_len == strlen(incoming_message.s_payload) + 1);
                assert(done_messages_count <= CHILDREN_COUNT);
                break;
            }
            case STOP: {
                stop_signal_received++;
                assert(incoming_message.s_header.s_payload_len == 0);
                assert(stop_signal_received == 1); // check stop signal received only once
                break;
            }
            default: {
                assert(0); // unreachable code
                break;
            }
        }
    }
}

void init_forks_array(Fork *forks, local_id forks_length, local_id child_id) {
    child_id--; // because indexing in array from 0
    for (local_id i = 0; i < forks_length; i++) {
        if (i == child_id)
            continue;
        // processes with smaller id have bigger priority (have forks)
        if (i < child_id) {
            forks[i] = (Fork) {.ownership = FO_NOT_OWNS, .state = FS_CLEAN, .request = FR_NOT_REQUESTED};
            continue;
        }
        forks[i] = (Fork) {.ownership = FO_OWNS, .state = FS_CLEAN, .request = FR_NOT_REQUESTED};
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
