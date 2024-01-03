#ifndef TASK_MSG_H
#define TASK_MSG_H
#include "event_loop.h"
struct task_msg {
    enum TASK_TYPE {
        NEW_CONN,
        NEW_TASK,
    };

    union {
        // NEW_CONN
        int connfd;

        // NEW_TASK
        struct {
            task_func func;
            void *args;
        };
    };

    TASK_TYPE type;
};
#endif