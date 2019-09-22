#ifndef __DC_LOG__H
#define __DC_LOG__H

void log_started(FILE *events_log_file, local_id process_id);

void log_received_all_started(FILE *events_log_file, local_id process_id);

void log_done(FILE *events_log_file, local_id process_id);

void log_received_all_done(FILE *events_log_file, local_id process_id);

void log_pipe_created(FILE *pipes_log_file, local_id from, local_id to, int read_fd, int write_fd);

void fatalf(char const *const fmt, ...);

#endif // __DC_LOG__H
