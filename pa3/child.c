#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>

#include "main.h"
#include "pa1.h"
#include "log.h"
#include "process_common.h"
#include "lamport.h"


void handle_transfer_requests(ProcessContext context);

void send_started(ProcessContext context);

void send_done(ProcessContext context);


void child_work(ProcessContext context) {
    close_unused_pipes(context.pipes, context.N, context.id);
    make_pipes_asynchronous(context.pipes, context.N, context.id);

    send_started(context);
    log_started(context.events_log_fd, context.id);

    receive_all_started(context);
    log_received_all_started(context.events_log_fd, context.id);

    handle_transfer_requests(context);

    close_process_pipes(context.pipes, context.N, context.id);
    fclose(context.events_log_fd);
}

void handle_transfer_requests(ProcessContext context) {
    const int children_count = context.N - 2; // minus parent and current process
    int stop_signal_received = 0;
    int done_messages_count = 0;

    while (1) {
        if (stop_signal_received && done_messages_count == children_count) {
            log_received_all_done(context.events_log_fd, context.id);
            return;
        }

        Message request;
        assert(receive_any(&context, &request) == 0);
        assert(request.s_header.s_magic == MESSAGE_MAGIC);
        lamport_receive_time(request.s_header.s_local_time);

        if (request.s_header.s_type == STOP) {
            stop_signal_received++;
            assert(request.s_header.s_payload_len == 0);
            assert(stop_signal_received == 1); // check stop signal received only once
            send_done(context);
            log_done(context.events_log_fd, context.id);
            continue;
        }

        if (request.s_header.s_type == DONE) {
            done_messages_count++;
            assert(request.s_header.s_payload_len == strlen(request.s_payload) + 1);
            assert(done_messages_count <= children_count);
            continue;
        }

        // below request's type must be transfer
        assert(request.s_header.s_type == TRANSFER);
        TransferOrder order;
        assert(sizeof(order) == request.s_header.s_payload_len);
        memcpy(&order, request.s_payload, request.s_header.s_payload_len);

        // TODO process balances

        if (order.s_src == context.id) {
            request.s_header.s_local_time = lamport_inc_get_time();
            assert(send(&context, order.s_dst, &request) == 0);
            continue;
        }

        // current process id must be equal to order's src or dst
        assert(order.s_dst == context.id);
        Message ack_for_parent = {.s_header = {
                .s_magic = MESSAGE_MAGIC, .s_payload_len = 0, .s_type = ACK, .s_local_time = lamport_inc_get_time()
        }};
        assert(send(&context, PARENT_ID, &ack_for_parent) == 0);
    }
}

void send_started(ProcessContext context) {
    MessageHeader header = {.s_magic = MESSAGE_MAGIC, .s_type = STARTED, .s_local_time = 0};
    Message msg = {.s_header = header};
    int length = sprintf(msg.s_payload, log_started_fmt, context.id, getpid(), getppid());
    msg.s_header.s_payload_len = length + 1;
    if (send_multicast(&context, &msg)) {
        debug_printf("could not send_multicast msg.\n");
    }
}

void send_done(ProcessContext context) {
    MessageHeader header = {.s_magic = MESSAGE_MAGIC, .s_type = DONE, .s_local_time = 0};
    Message msg = {.s_header = header};
    int length = sprintf(msg.s_payload, log_done_fmt, context.id);
    msg.s_header.s_payload_len = length + 1;
    if (send_multicast(&context, &msg)) {
        debug_printf("could not send_multicast msg.\n");
    }
}
