#ifndef EVENT_LOOP_H
#define EVENT_LOOP_H

#include "event_base.h"
#include <sys/epoll.h>
#include <unordered_map>
#include <unordered_set>

#define MAXEVENTS 10
using io_event_map_it = std::unordered_map<int, io_event>::iterator;
class event_loop {
  public:
    event_loop();
    void event_process();
    void add_io_event(int fd, io_callback *proc, int mask,
                      void *args = nullptr);

    void del_io_event(int fd);
    void del_io_event(int fd, int mask);

  private:
    int _epfd;
    std::unordered_map<int, io_event> _io_evs;
    std::unordered_set<int> _listen_fds;

    // 一次性最大处理的事件
    epoll_event _fired_evs[MAXEVENTS];
};
#endif