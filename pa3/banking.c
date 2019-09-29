#include <assert.h>

#include "banking.h"
#include "log.h"
#include "string.h"


void transfer(void *parent_data, local_id src, local_id dst, balance_t amount) {
    debug_printf("transfer from src %d to dst %d amount %d.\n", src, dst, amount);
    TransferOrder order = {.s_src = src, .s_dst = dst, .s_amount = amount};
    MessageHeader header = {
            .s_magic = MESSAGE_MAGIC,
            .s_payload_len = sizeof(order),
            .s_type = TRANSFER,
            .s_local_time = get_lamport_time()
    };
    Message transfer_message = {.s_header = header};
    memcpy(transfer_message.s_payload, &order, sizeof(order));
    assert(send(parent_data, src, &transfer_message) == 0);

    Message ack_message;
    assert(receive(parent_data, dst, &ack_message) == 0);
    assert(ack_message.s_header.s_magic == MESSAGE_MAGIC);
    assert(ack_message.s_header.s_type == ACK);
    assert(ack_message.s_header.s_payload_len == 0);
}

timestamp_t get_lamport_time() {
    // function implemented in lamport.c
    // do not call this func
    assert(0);
    return 0;
}
