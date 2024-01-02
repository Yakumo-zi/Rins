#include "thread_pool.h"
#include "event_base.h"
#include "task_msg.h"
#include "tcp_conn.h"
#include "thread_queue.h"
#include <cstdio>
#include <netinet/in.h>
#include <pthread.h>
#include <queue>

void deal_task_message(event_loop *loop, int fd, void *args) {
    thread_queue<task_msg> *queue = (thread_queue<task_msg> *)args;
    std::queue<task_msg> tasks;
    queue->recv(tasks);
    while (!tasks.empty()) {
        task_msg task = tasks.front();
        tasks.pop();
        if (task.type == task_msg::NEW_CONN) {
            tcp_conn *conn = new tcp_conn(task.connfd, loop);
            if (conn == nullptr) {
                fprintf(stderr, "in thread new tcp_conn error\n");
                exit(1);
            }
            printf("[thread]: get new connection succ!\n");
        } else if (task.type == task_msg::NEW_TASK) {

        } else {
            fprintf(stderr, "unknow task!\n");
        }
    }
}

void *thread_main(void *args) {
    thread_queue<task_msg> *queue = (thread_queue<task_msg> *)args;
    event_loop *loop = new event_loop();
    if (loop == nullptr) {
        fprintf(stderr, "new event_loop error\n");
        exit(1);
    }

    queue->set_loop(loop);
    queue->set_callback(deal_task_message, queue);

    loop->event_process();
    return nullptr;
}

thread_pool::thread_pool(int thread_cnt) {
    _index = 0;
    _queues = nullptr;
    _thread_cnt = thread_cnt;
    if (_thread_cnt <= 0) {
        fprintf(stderr, "thread_pool::constructor _thread_cnt < 0\n");
        exit(1);
    }

    _queues = new thread_queue<task_msg> *[_thread_cnt];
    _tids = new pthread_t[_thread_cnt];
    int ret;
    for (int i = 0; i < thread_cnt; i++) {
        printf("create %d thread\n", i);
        _queues[i] = new thread_queue<task_msg>();
        ret = pthread_create(&_tids[i], nullptr, thread_main, _queues[i]);
        if (ret == -1) {
            perror("thread_pool, create thread\n");
            exit(1);
        }
        pthread_detach(_tids[i]);
    }
}

thread_queue<task_msg> *thread_pool::get_thread() {
    if (_index == _thread_cnt) {
        _index = 0;
    }
    return _queues[_index++];
}
