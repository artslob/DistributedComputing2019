#include "ipc.h"
#include "main.h"


timestamp_t lamport_inc_get_time(ProcessContext *context) {
    context->time++;
    return context->time;
}

timestamp_t lamport_receive_time(ProcessContext *context, timestamp_t received_time) {
    context->time = (context->time > received_time) ? context->time : received_time;
    return lamport_inc_get_time(context);
}
