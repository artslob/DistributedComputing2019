#include "ipc.h"


static timestamp_t lamport_time = 0;

timestamp_t lamport_get_time() {
    return lamport_time;
}

timestamp_t lamport_inc_get_time() {
    lamport_time++;
    return lamport_get_time();
}

timestamp_t lamport_receive_time(timestamp_t received_time) {
    lamport_time = (lamport_time > received_time) ? lamport_time : received_time;
    return lamport_inc_get_time();
}
