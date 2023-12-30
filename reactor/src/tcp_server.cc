#include "tcp_server.h"
#include "buf_pool.h"
#include "event_base.h"
#include "reactor_buf.h"
#include "tcp_conn.h"
#include <arpa/inet.h>
#include <asm-generic/errno-base.h>
#include <asm-generic/socket.h>
#include <bits/stdint-uintn.h>
#include <cstdlib>
#include <errno.h>
#include <memory>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

void accept_callback(event_loop *loop, int fd, void *args) {
    tcp_server *server = (tcp_server *)args;
    server->do_accept();
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
    loop->add_io_event(_sock_fd, accept_callback, EPOLLIN, this);
}

struct message {
    char data[m4K];
    char len;
};
message msg;
void server_rd_callback(event_loop *loop, int fd, void *args);
void server_wt_callback(event_loop *loop, int fd, void *args);
void server_rd_callback(event_loop *loop, int fd, void *args) {
    int ret = 0;
    message *msg = (message *)args;
    input_buf ibuf;
    ret = ibuf.read_data(fd);
    if (ret == -1) {
        fprintf(stderr, "tcp_server::rd_callback ibuf read_data error\n");
        //删除事件
        loop->del_io_event(fd);
        //对端关闭
        close(fd);
        return;
    }
    if (ret == 0) {
        loop->del_io_event(fd);
        close(fd);
        return;
    }
    printf("tcp_server::rd_callbcak ibuf.length()=%d\n", ibuf.length());
    msg->len = ibuf.length();
    bzero(msg->data, msg->len);
    memcpy(msg->data, ibuf.data(), msg->len);
    ibuf.pop(msg->len);
    ibuf.adjust();
    printf("tcp_server::rd_callback recv data = %s\n", msg->data);
    loop->del_io_event(fd, EPOLLIN);
    loop->add_io_event(fd, server_wt_callback, EPOLLOUT, msg);
}

void server_wt_callback(event_loop *loop, int fd, void *args) {
    message *msg = (message *)args;
    output_buf obuf;
    obuf.send_data(msg->data, msg->len);
    while (obuf.length()) {
        int write_ret = obuf.write2fd(fd);
        if (write_ret == -1) {
            fprintf(stderr,
                    "tcp_server::wt_callback obuf write connfd error\n");
            return;
        } else if (write_ret == 0) {
            break;
        }
    }
    bzero(msg, msg->len);
    loop->del_io_event(fd, EPOLLOUT);
    loop->add_io_event(fd, server_rd_callback, EPOLLIN, msg);
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
            tcp_conn *conn = new tcp_conn(connfd, _event_loop);
            if (conn == nullptr) {
                fprintf(stderr, "tcp_server::accept new tcp_conn error\n");
                exit(1);
            }
            printf("tcp_server::accept get new connection succ!\n");
            break;
        }
    }
}
tcp_server::~tcp_server() { close(_sock_fd); }