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

        //NEW_TASK
        struct {
            void (*task_cb)(event_loop *, void *args);
            void *args;
        };
    };

    TASK_TYPE type;
};
#endif