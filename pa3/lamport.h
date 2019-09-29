#ifndef __DC__LAMPORT__H
#define __DC__LAMPORT__H

#include "ipc.h"
#include "main.h"


timestamp_t lamport_inc_get_time(ProcessContext *context);

timestamp_t lamport_receive_time(ProcessContext *context, timestamp_t received_time);

#endif //__DC__LAMPORT__H
