#pragma once

#include "rins.pb.h"
#include "rins_reactor.h"
#include "route_lb.h"

//与report_client通信的thread_queue消息队列
extern thread_queue<rins::ReportStatusRequest> *report_queue;

//与dns_client通信的thread_queue消息队列
extern thread_queue<rins::GetRouteRequest> *dns_queue;

extern route_lb *r_lb[3];

// 启动udp server服务,用来接收业务层(调用者/使用者)的消息
void start_UDP_servers(void);

// 启动lars_reporter client 线程
void start_report_client(void);

// 启动lars_dns client 线程
void start_dns_client(void);