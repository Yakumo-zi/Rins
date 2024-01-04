#pragma once

#include "event_loop.h"
#include "rins.pb.h"
#include "rins_reactor.h"
#include "store_report.h"
#include "thread_queue.h"
#include <queue>
#include <utility>
using Args =
    std::pair<thread_queue<rins::ReportStatusRequest> *, store_report *>;

void thread_report(event_loop *loop, int fd, void *args) {
    thread_queue<rins::ReportStatusRequest> *queue = ((Args *)args)->first;
    store_report *sr = ((Args *)args)->second;

    std::queue<rins::ReportStatusRequest> report_msgs;
    queue->recv(report_msgs);
    while (!report_msgs.empty()) {
        auto msg = report_msgs.front();
        report_msgs.pop();
        sr->store(msg);
    }
}

void *store_main(void *args) {
    thread_queue<rins::ReportStatusRequest> *queue =
        (thread_queue<rins::ReportStatusRequest> *)args;
    event_loop loop;
    store_report sr;
    Args cb_args;
    cb_args.first = queue;
    cb_args.second = &sr;
    queue->set_loop(&loop);
    queue->set_callback(thread_report, &cb_args);

    //启动事件监听
    loop.event_process();
    return nullptr;
}

void *store_main(void *args);