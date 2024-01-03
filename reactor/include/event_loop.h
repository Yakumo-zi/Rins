#ifndef EVENT_LOOP_H
#define EVENT_LOOP_H

#include "event_base.h"
#include <sys/epoll.h>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#define MAXEVENTS 10
using io_event_map_it = std::unordered_map<int, io_event>::iterator;

// 该任务需要对当前 event_loop 中所监听的所有文件描述符进行操作

typedef void (*task_func)(event_loop *loop, void *args);
class event_loop {
  public:
    event_loop();
    void event_process();
    void add_io_event(int fd, io_callback *proc, int mask,
                      void *args = nullptr);

    void del_io_event(int fd);
    void del_io_event(int fd, int mask);
    void get_listen_fds(std::unordered_set<int> &fds) { fds = _listen_fds; }

    void add_task(task_func func, void *args);
    void execute_ready_task();

  private:
    int _epfd;
    std::unordered_map<int, io_event> _io_evs;
    std::unordered_set<int> _listen_fds;

    // 一次性最大处理的事件
    epoll_event _fired_evs[MAXEVENTS];
    std::vector<std::pair<task_func, void *>> _read_tasks;
};
#endif