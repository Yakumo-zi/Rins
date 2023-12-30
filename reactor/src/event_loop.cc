#include "event_loop.h"
#include "event_base.h"
#include <cassert>
#include <cstdio>
#include <sys/epoll.h>
#include <sys/ucontext.h>

event_loop::event_loop() {
    _epfd = epoll_create(255);
    if (_epfd == -1) {
        fprintf(stderr, "event_loop::constructor epoll create error\n");
        exit(1);
    }
}

// 阻塞循环处理事件
void event_loop::event_process() {
    while (true) {
        io_event_map_it ev_it;
        int nfds = epoll_wait(_epfd, _fired_evs, MAXEVENTS, 10);
        for (int i = 0; i < nfds; i++) {
            ev_it = _io_evs.find(_fired_evs[i].data.fd);
            assert(ev_it != _io_evs.end());
            io_event *ev = &(ev_it->second);
            if (_fired_evs[i].events & EPOLLIN) {
                void *args = ev->rcb_args;
                ev->read_callback(this, _fired_evs[i].data.fd, args);
            } else if (_fired_evs[i].events & EPOLLOUT) {
                void *args = ev->wcb_args;
                ev->write_callback(this, _fired_evs[i].data.fd, args);
            } else if (_fired_evs[i].events & (EPOLLHUP | EPOLLERR)) {
                // 水平触发未处理，可能会出现HUP事件，正常读写处理，没有则清空
                if (ev->read_callback != nullptr) {
                    void *args = ev->rcb_args;
                    ev->read_callback(this, _fired_evs[i].data.fd, args);
                } else if (ev->write_callback != nullptr) {

                    void *args = ev->wcb_args;
                    ev->write_callback(this, _fired_evs[i].data.fd, args);
                } else {
                    fprintf(stderr,
                            "event_pool::event_process fd %d get error, delete "
                            "it from epoll\n",
                            _fired_evs[i].data.fd);
                    this->del_io_event(_fired_evs[i].data.fd);
                }
            }
        }
    }
}

void event_loop::add_io_event(int fd, io_callback *proc, int mask, void *args) {
    int final_mask;
    int op;
    io_event_map_it it = _io_evs.find(fd);
    if (it == _io_evs.end()) {
        final_mask = mask;
        op = EPOLL_CTL_ADD;
    } else {
        final_mask = it->second.mask | mask;
        op = EPOLL_CTL_MOD;
    }

    if (mask & EPOLLIN) {
        _io_evs[fd].read_callback = proc;
        _io_evs[fd].rcb_args = args;
    } else if (mask & EPOLLOUT) {
        _io_evs[fd].write_callback = proc;
        _io_evs[fd].wcb_args = args;
    }

    _io_evs[fd].mask = final_mask;
    epoll_event event;
    event.events = final_mask;
    event.data.fd = fd;
    if (epoll_ctl(_epfd, op, fd, &event) == -1) {
        fprintf(stderr, "event_loop::add_io_event epoll ctl %d error\n", fd);
        return;
    }
    _listen_fds.insert(fd);
}

void event_loop::del_io_event(int fd) {
    //将事件从_io_evs删除
    _io_evs.erase(fd);
    //将fd从监听集合中删除
    _listen_fds.erase(fd);
    //将fd从epoll堆删除
    epoll_ctl(_epfd, EPOLL_CTL_DEL, fd, NULL);
}

//删除一个io事件的EPOLLIN/EPOLLOUT
void event_loop::del_io_event(int fd, int mask) {
    //如果没有该事件，直接返回
    io_event_map_it it = _io_evs.find(fd);
    if (it == _io_evs.end()) {
        return;
    }

    int &o_mask = it->second.mask;
    //修正mask
    o_mask = o_mask & (~mask);

    if (o_mask == 0) {
        //如果修正之后 mask为0，则删除
        this->del_io_event(fd);
    } else {
        //如果修正之后，mask非0，则修改
        struct epoll_event event;
        event.events = o_mask;
        event.data.fd = fd;
        epoll_ctl(_epfd, EPOLL_CTL_MOD, fd, &event);
    }
}