#ifndef THREAD_POOL_H
#define THREAD_POOL_H
#include "event_loop.h"
#include "task_msg.h"
#include "thread_queue.h"
class thread_pool {
  public:
    thread_pool(int thread_cnt);
    thread_queue<task_msg> *get_thread();
    void send_task(task_func func, void *args);

  private:
    thread_queue<task_msg> **_queues;
    int _thread_cnt;
    pthread_t *_tids;
    int _index;
};

#endif