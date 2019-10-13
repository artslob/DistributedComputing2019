#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>

#include "pipes.h"
#include "log.h"
#include "process_common.h"
#include "lamport.h"
#include "ipc.h"
#include "pa2345.h"
#include "child.h"


static void handle_requests(ProcessContext context);

static void handle_requests_without_cs(ProcessContext context);

static void send_started(ProcessContext context);

static void send_done(ProcessContext context);


void child_work(ProcessContext context) {
    close_unused_pipes(context.pipes, context.N, context.id);

    send_started(context);
    log_started(context.events_log_fd, context.id);

    receive_all_started(context);
    log_received_all_started(context.events_log_fd, context.id);

    if (context.mutexl) {
        handle_requests(context);
    } else {
        handle_requests_without_cs(context);
    }

    close_process_pipes(context.pipes, context.N, context.id);
    fclose(context.events_log_fd);
}

/** Передача маркера запроса
 * Процесс отправляет соседнему процессу маркер запроса, если данный процесс:
 * 1. Владеет маркером;
 * 2. Не владеет вилкой;
 * 3. Процесс хочет зайти в critical section.
 */
int fork_can_be_requested(const Fork fork) {
    return fork.ownership == FO_NOT_OWNS && fork.request == FR_TOKEN;
}

static int should_send_request(ProcessContext context) {
    for (local_id i = 1; i < context.forks_length; i++) {
        if (i == context.id)
            continue;
        if (fork_can_be_requested(context.forks[i]))
            return 1;
    }
    return 0;
}

/** Переход из ожидания в critical section (CS)
 * Ожидающий процесс может войти в CS только тогда, когда он владеет всеми вилками своих соседей
 * и для любого его соседа верно, что общая вилка чистая либо у неё нет маркера запроса от соседа.
 * Когда процесс переходит в CS, все его вилки становятся грязными.
 */
static int can_enter_cs(ProcessContext context) {
    for (local_id i = 1; i < context.forks_length; i++) {
        if (i == context.id)
            continue;
        Fork current_fork = context.forks[i];
        int is_owns_fork = current_fork.ownership == FO_OWNS;
        int clean_or_no_token = current_fork.state == FS_CLEAN || current_fork.request == FR_MISSING_TOKEN;
        if (is_owns_fork && clean_or_no_token)
            continue;
        return 0;
    }
    return 1;
}

/** Передача вилки
 * Процесс пересылает вилку, если данный процесс:
 * 1. Имеет вместе с этой вилкой соответствующий маркер запроса;
 * 2. Вилка грязная;
 * 3. Процессу не нужно входить в critical section.
 * При передаче вилка очищается.
 */
int fork_can_be_released(const Fork fork) {
    return fork.ownership == FO_OWNS && fork.state == FS_DIRTY && fork.request == FR_TOKEN;
}

static int should_release_fork(ProcessContext context) {
    for (local_id i = 1; i < context.forks_length; i++) {
        if (i == context.id)
            continue;
        if (fork_can_be_released(context.forks[i]))
            return 1;
    }
    return 0;
}

static void mark_forks_as_dirty(ProcessContext *context) {
    for (local_id i = 1; i < context->forks_length; i++) {
        if (i == context->id)
            continue;
        context->forks[i].state = FS_DIRTY;
    }
}

static void handle_requests(ProcessContext context) {
    const int CHILDREN_COUNT = context.N - 2; // minus parent and current process
    const int MAX_LOOP_COUNT = context.id * 5;
    int stop_signal_received = 0;
    int done_messages_count = 0;
    int loop_iteration = 1;
    int done_sent = 0;

    while (1) {
        if (stop_signal_received && done_messages_count == CHILDREN_COUNT) {
            log_received_all_done(context.events_log_fd, context.id);
            return;
        }

        if (done_sent == 0 && loop_iteration > MAX_LOOP_COUNT) {
            send_done(context);
            log_done(context.events_log_fd, context.id);
            done_sent = 1;
            continue;
        }

        if (loop_iteration <= MAX_LOOP_COUNT && should_send_request(context)) {
            request_cs(&context);
            continue;
        }

        if (should_release_fork(context)) {
            release_cs(&context);
            continue;
        }

        Message incoming_message;
        int receive_any_status = receive_any(&context, &incoming_message);

        if (receive_any_status == -1) {
            // -1 means no messages
            if (loop_iteration <= MAX_LOOP_COUNT && can_enter_cs(context)) {
                log_loop_operation(context.id, loop_iteration++, MAX_LOOP_COUNT);
                mark_forks_as_dirty(&context);
            }
            continue;
        }

        assert(receive_any_status == 0);
        assert(incoming_message.s_header.s_magic == MESSAGE_MAGIC);
        lamport_receive_time(incoming_message.s_header.s_local_time);

        switch (incoming_message.s_header.s_type) {
            case STOP: {
                stop_signal_received++;
                assert(incoming_message.s_header.s_payload_len == 0);
                assert(stop_signal_received == 1); // check stop signal received only once
                break;
            }
            case DONE: {
                done_messages_count++;
                assert(incoming_message.s_header.s_payload_len == strlen(incoming_message.s_payload) + 1);
                assert(done_messages_count <= CHILDREN_COUNT);
                break;
            }
            case CS_REPLY: {
                assert(incoming_message.s_header.s_payload_len == 0);
                break;
            }
            case CS_REQUEST: {
                Request request;
                assert(incoming_message.s_header.s_payload_len == sizeof(request));
                memcpy(&request, incoming_message.s_payload, incoming_message.s_header.s_payload_len);
                context.forks[request.from].request = FR_TOKEN;
                break;
            }
            case CS_RELEASE: {
                Request request;
                assert(incoming_message.s_header.s_payload_len == sizeof(request));
                memcpy(&request, incoming_message.s_payload, incoming_message.s_header.s_payload_len);
                context.forks[request.from].ownership = FO_OWNS;
                context.forks[request.from].state = FS_CLEAN;
                break;
            }
            default: {
                assert(0); // unreachable code
                break;
            }
        }
    }
}

static void handle_requests_without_cs(ProcessContext context) {
    const int CHILDREN_COUNT = context.N - 2; // minus parent and current process
    const int MAX_LOOP_COUNT = context.id * 5;
    int stop_signal_received = 0;
    int done_messages_count = 0;

    for (int i = 1; i <= MAX_LOOP_COUNT; i++) {
        log_loop_operation(context.id, i, MAX_LOOP_COUNT);
    }
    send_done(context);
    log_done(context.events_log_fd, context.id);

    while (1) {
        if (stop_signal_received && done_messages_count == CHILDREN_COUNT) {
            log_received_all_done(context.events_log_fd, context.id);
            return;
        }

        Message incoming_message;
        assert(receive_any(&context, &incoming_message) == 0);
        assert(incoming_message.s_header.s_magic == MESSAGE_MAGIC);
        lamport_receive_time(incoming_message.s_header.s_local_time);

        switch (incoming_message.s_header.s_type) {
            case DONE: {
                done_messages_count++;
                assert(incoming_message.s_header.s_payload_len == strlen(incoming_message.s_payload) + 1);
                assert(done_messages_count <= CHILDREN_COUNT);
                break;
            }
            case STOP: {
                stop_signal_received++;
                assert(incoming_message.s_header.s_payload_len == 0);
                assert(stop_signal_received == 1); // check stop signal received only once
                break;
            }
            default: {
                assert(0); // unreachable code
                break;
            }
        }
    }
}

void init_forks_array(Fork *forks, local_id forks_length, local_id child_id) {
    for (local_id i = 1; i < forks_length; i++) {
        if (i == child_id)
            continue;
        // processes with smaller id have bigger priority (have forks)
        if (i < child_id) {
            forks[i] = (Fork) {.ownership = FO_NOT_OWNS, .state = FS_CLEAN, .request = FR_TOKEN};
            continue;
        }
        forks[i] = (Fork) {.ownership = FO_OWNS, .state = FS_CLEAN, .request = FR_MISSING_TOKEN};
    }
}

static void send_started(ProcessContext context) {
    Message msg = {.s_header = {
            .s_magic = MESSAGE_MAGIC, .s_type = STARTED, .s_local_time = lamport_inc_get_time()
    }};
    int length = sprintf(msg.s_payload, log_started_fmt, lamport_get_time(), context.id, getpid(), getppid(), 0);
    msg.s_header.s_payload_len = length + 1;
    assert(send_multicast(&context, &msg) == 0);
}

static void send_done(ProcessContext context) {
    Message msg = {.s_header = {
            .s_magic = MESSAGE_MAGIC, .s_type = DONE, .s_local_time = lamport_inc_get_time()
    }};
    int length = sprintf(msg.s_payload, log_done_fmt, lamport_get_time(), context.id, 0);
    msg.s_header.s_payload_len = length + 1;
    assert(send_multicast(&context, &msg) == 0);
}
