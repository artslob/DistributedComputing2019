#include "banking.h"
#include "log.h"


void transfer(void *parent_data, local_id src, local_id dst, balance_t amount) {
    debug_printf("transfer from src %d to dst %d amount %d.\n", src, dst, amount);
}
