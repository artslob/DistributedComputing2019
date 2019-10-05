#ifndef __DC__LAMPORT__H
#define __DC__LAMPORT__H

#include "ipc.h"
#include "main.h"

timestamp_t lamport_get_time();

timestamp_t lamport_inc_get_time();

timestamp_t lamport_receive_time(timestamp_t received_time);

#endif //__DC__LAMPORT__H
