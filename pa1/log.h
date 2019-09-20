#ifndef __DC_LOG__H
#define __DC_LOG__H

void log_pipe_created(int pipes_log_fd, int from, int to, int read_fd, int write_fd);
void fatalf(char const * const fmt, ...);

#endif // __DC_LOG__H
