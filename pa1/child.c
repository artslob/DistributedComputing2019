#include <stdio.h>
#include <unistd.h>

#include "main.h"
#include "pa1.h"
#include "log.h"
#include "process_common.h"


void send_started(ProcessContext context);

void send_done(ProcessContext context);


void child_work(ProcessContext context) {
    close_unused_pipes(context.pipes, context.N, context.id);

    send_started(context);
    log_started(context.events_log_fd, context.id);

    receive_all_started(context);
    log_received_all_started(context.events_log_fd, context.id);

    send_done(context);
    log_done(context.events_log_fd, context.id);

    receive_all_done(context);
    log_received_all_done(context.events_log_fd, context.id);

    close_process_pipes(context.pipes, context.N, context.id);
    fclose(context.events_log_fd);
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
