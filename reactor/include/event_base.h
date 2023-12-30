#ifndef EVENT_BASE_H
#define EVENT_BASE_H
class event_loop;

typedef void io_callback(event_loop *loop, int fd, void *args);

struct io_event {
    io_event()
        : read_callback(nullptr), write_callback(nullptr), rcb_args(nullptr),
          wcb_args(nullptr) {}
    int mask;
    io_callback *read_callback;
    io_callback *write_callback;
    void *rcb_args;
    void *wcb_args;
};
#endif