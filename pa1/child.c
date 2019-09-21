#include <stdio.h>
#include <unistd.h>

#include "main.h"
#include "pa1.h"


void child_work(ProcessContext context) {
    close_unused_pipes(context.pipes, context.N, context.id);

    MessageHeader header = {.s_magic = MESSAGE_MAGIC, .s_type = STARTED, .s_local_time = 0};
    Message msg = {.s_header = header};
    int length = sprintf(msg.s_payload, log_started_fmt, context.id, getpid(), getppid());
    msg.s_header.s_payload_len = length + 1;
    if (send_multicast(&context, &msg)) {
        printf("could not send_multicast msg.\n");
    }
    receive_all_started(context);

    header = (MessageHeader) {.s_magic = MESSAGE_MAGIC, .s_type = DONE, .s_local_time = 0};
    msg = (Message) {.s_header = header};
    length = sprintf(msg.s_payload, log_done_fmt, context.id);
    msg.s_header.s_payload_len = length + 1;
    if (send_multicast(&context, &msg)) {
        printf("could not send_multicast msg.\n");
    }
    receive_all_done(context);

    close_process_pipes(context.pipes, context.N, context.id);
    close(context.events_log_fd);
}
