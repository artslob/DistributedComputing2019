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

static void handle_requests(ProcessContext context) {
    const int children_count = context.N - 2; // minus parent and current process
    int done_messages_count = 0;

    useful_work(context);
    send_done(context);
    log_done(context.events_log_fd, context.id);

    while (1) {
        if (done_messages_count == children_count) {
            log_received_all_done(context.events_log_fd, context.id);
            return;
        }

        Message request;
        assert(receive_any(&context, &request) == 0);
        assert(request.s_header.s_magic == MESSAGE_MAGIC);
        lamport_receive_time(request.s_header.s_local_time);

        if (request.s_header.s_type == DONE) {
            done_messages_count++;
            assert(request.s_header.s_payload_len == strlen(request.s_payload) + 1);
            assert(done_messages_count <= children_count);
            continue;
        }
    }
}

static void useful_work(ProcessContext context) {
    const int LOOP_COUNT = context.id * 5;
    for (int i = 0; i < LOOP_COUNT; i++) {
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
