#ifndef THREAD_POOL_H
#define THREAD_POOL_H
#include "thread_queue.h"
#include "task_msg.h"
class thread_pool{
    public:
    thread_pool(int thread_cnt);
    thread_queue<task_msg>* get_thread();
    private:
    thread_queue<task_msg>** _queues;
    int _thread_cnt;
    pthread_t* _tids;
    int _index;
};

#endif