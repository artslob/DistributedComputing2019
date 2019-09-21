#ifndef __DC_LOG__H
#define __DC_LOG__H

void log_started(int events_log_fd, local_id process_id);
void log_received_all_started(int events_log_fd, local_id process_id);
void log_done(int events_log_fd, local_id process_id);
void log_received_all_done(int events_log_fd, local_id process_id);

void log_pipe_created(int pipes_log_fd, int from, int to, int read_fd, int write_fd);
void fatalf(char const * const fmt, ...);

#endif // __DC_LOG__H
