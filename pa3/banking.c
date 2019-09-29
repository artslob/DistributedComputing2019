#include <assert.h>

#include "banking.h"
#include "log.h"
#include "string.h"
#include "lamport.h"


void transfer(void *parent_data, local_id src, local_id dst, balance_t amount) {
    debug_printf("transfer from src %d to dst %d amount %d.\n", src, dst, amount);
    TransferOrder order = {.s_src = src, .s_dst = dst, .s_amount = amount};
    Message transfer_message = {.s_header = {
            .s_magic = MESSAGE_MAGIC,
            .s_payload_len = sizeof(order),
            .s_type = TRANSFER,
            .s_local_time = lamport_inc_get_time()
    }};
    memcpy(transfer_message.s_payload, &order, sizeof(order));
    assert(send(parent_data, src, &transfer_message) == 0);

    Message ack_message;
    assert(receive(parent_data, dst, &ack_message) == 0);
    assert(ack_message.s_header.s_magic == MESSAGE_MAGIC);
    assert(ack_message.s_header.s_type == ACK);
    assert(ack_message.s_header.s_payload_len == 0);
    lamport_receive_time(ack_message.s_header.s_local_time);
}
