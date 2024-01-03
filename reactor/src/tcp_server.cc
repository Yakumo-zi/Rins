#include "tcp_server.h"
#include "buf_pool.h"
#include "config_file.h"
#include "event_base.h"
#include "message.h"
#include "net_connection.h"
#include "reactor_buf.h"
#include "task_msg.h"
#include "tcp_conn.h"
#include "thread_pool.h"
#include "thread_queue.h"
#include <arpa/inet.h>
#include <asm-generic/errno-base.h>
#include <asm-generic/socket.h>
#include <bits/stdint-uintn.h>
#include <cstdlib>
#include <errno.h>
#include <memory>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

tcp_conn **tcp_server::conns = nullptr;
int tcp_server::_max_conns = 0;
int tcp_server::_cur_conns = 0;
pthread_mutex_t tcp_server::_conns_mutex = PTHREAD_MUTEX_INITIALIZER;
msg_router tcp_server::router;
conn_callback tcp_server::conn_close_cb = nullptr;
void *tcp_server::conn_close_cb_args = nullptr;

conn_callback tcp_server::conn_start_cb = nullptr;
void *tcp_server::conn_start_cb_args = nullptr;

void tcp_server::increase_conn(int connfd, tcp_conn *conn) {
    pthread_mutex_lock(&_conns_mutex);
    conns[connfd] = conn;
    _cur_conns++;
    pthread_mutex_unlock(&_conns_mutex);
}
void accept_callback(event_loop *loop, int fd, void *args) {
    tcp_server *server = (tcp_server *)args;
    server->do_accept();
}
void tcp_server::decrease_conn(int connfd) {
    pthread_mutex_lock(&_conns_mutex);
    conns[connfd] = NULL;
    _cur_conns--;
    pthread_mutex_unlock(&_conns_mutex);
}

//得到当前链接的刻度
void tcp_server::get_conn_num(int *curr_conn) {
    pthread_mutex_lock(&_conns_mutex);
    *curr_conn = _cur_conns;
    pthread_mutex_unlock(&_conns_mutex);
}

tcp_server::tcp_server(event_loop *loop, const char *ip, uint16_t port) {

    bzero(&_in_connaddr, sizeof(_in_connaddr));

    if (signal(SIGHUP, SIG_IGN) == SIG_ERR) {
        fprintf(stderr, "signal ignore SIGHUP\n");
    }
    if (signal(SIGPIPE, SIG_IGN) == SIG_ERR) {
        fprintf(stderr, "signal ignore SIGPIPE\n");
    }

    _sock_fd = socket(AF_INET, SOCK_STREAM | SOCK_CLOEXEC, IPPROTO_TCP);
    if (_sock_fd == -1) {
        fprintf(stderr, "tcp_server::socket\n");
        exit(1);
    }

    sockaddr_in server_addr;
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    inet_aton(ip, &server_addr.sin_addr);

    int op = 1;

    if (setsockopt(_sock_fd, SOL_SOCKET, SO_REUSEADDR, &op, sizeof(op)) < 0) {
        fprintf(stderr, "tcp_server::setsocketopt\n");
    }
    if (bind(_sock_fd, (const sockaddr *)&server_addr, sizeof(server_addr)) ==
        -1) {
        fprintf(stderr, "tcpserver::bind %s\n", strerror(errno));
        exit(1);
    }

    if (listen(_sock_fd, 500) == -1) {
        fprintf(stderr, "tcpserver::listen\n");
        exit(1);
    }
    _event_loop = loop;

    _max_conns = config_file::instance()->get_number("reactor", "maxConn",1024);
    conns = new tcp_conn *[_max_conns + 3];
    if (conns == nullptr) {
        fprintf(stderr, "tcp_server::constructor new conns[%d] error\n",
                _max_conns);
        exit(1);
    }

    int thread_cnt = config_file::instance()->get_number("reactor", "threadNum",3);
    if (thread_cnt > 0) {
        _thread_pool = new thread_pool(thread_cnt);
        if (_thread_pool == nullptr) {
            fprintf(stderr, "tcp_server::constructor new thread_pool error\n");
            exit(1);
        }
    }

    _event_loop->add_io_event(_sock_fd, accept_callback, EPOLLIN, this);
}

void tcp_server::do_accept() {
    int connfd;
    while (true) {
        printf("tcp_server::accept\n");
        connfd = accept(_sock_fd, (sockaddr *)&_in_connaddr, &_addrlen);

        if (connfd == -1) {
            if (errno == EINTR) {
                fprintf(stderr, "tcp_server::accept errno=EINTR\n");
                continue;
            } else if (errno == EMFILE) {
                // 文件描述符用尽
                fprintf(stderr, "tcp_server::accept errno=EMFILE\n");
            } else if (errno == EAGAIN) {
                fprintf(stderr, "tcp_server::accept errno=EAGIN\n");
                break;
            } else {
                fprintf(stderr, "tcp_server::accept other errors\n");
            }
        } else {
            int cur_conns;
            get_conn_num(&cur_conns);
            if (cur_conns >= _max_conns) {
                fprintf(stderr,
                        "tcp_server::accept so many connections, max = %d\n",
                        _max_conns);
                close(connfd);
            } else {
                if (_thread_pool != nullptr) {
                    thread_queue<task_msg> *queue = _thread_pool->get_thread();
                    task_msg task;
                    task.type = task_msg::NEW_CONN;
                    task.connfd = connfd;
                    queue->send(task);
                } else {
                    tcp_conn *conn = new tcp_conn(connfd, _event_loop);
                    if (conn == NULL) {
                        fprintf(stderr,
                                "tcp_server::accept new tcp_conn error\n");
                        exit(1);
                    }
                    printf("tcp_server::accept get new connection succ!\n");
                }
            }
            break;
        }
    }
}
tcp_server::~tcp_server() { close(_sock_fd); }