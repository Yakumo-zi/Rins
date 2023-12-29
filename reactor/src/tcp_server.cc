#include "tcp_server.h"
#include "reactor_buf.h"
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
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

tcp_server::tcp_server(const char *ip, uint16_t port) {

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
    if (bind(_sock_fd, (const sockaddr *)&server_addr, sizeof(server_addr)) <
        0) {
        fprintf(stderr, "tcpserver::bind\n");
        exit(1);
    }

    if (listen(_sock_fd, 500) == -1) {
        fprintf(stderr, "tcpserver::listen\n");
        exit(1);
    }
}

void tcp_server::do_accept() {
    int connfd;
    while (true) {
        printf("tcp_server::accept begin\n");
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
            int ret = 0;
            input_buf ibuf;
            output_buf obuf;
            char *msg = nullptr;
            int msg_len = 0;
            do {
                ret = ibuf.read_data(connfd);
                if (ret == -1) {
                    fprintf(stderr, "tcp_server::serve ibuf read_data error\n");
                    break;
                }
                printf("tcp_server::serve ibuf.length()=%d\n", ibuf.length());
                msg_len = ibuf.length();
                msg = (char *)malloc(msg_len);
                bzero(msg, msg_len);
                memcpy(msg, ibuf.data(), msg_len);
                ibuf.pop(msg_len);
                ibuf.adjust();
                printf("tcp_server::serve recv data = %s\n", msg);
                obuf.send_data(msg, msg_len);
                while (obuf.length()) {
                    int write_ret = obuf.write2fd(connfd);
                    if (write_ret == -1) {
                        fprintf(stderr,
                                "tcp_server::serve obuf write connfd error\n");
                        return;
                    } else if (write_ret == 0) {
                        break;
                    }
                }
                free(msg);
            } while (ret != 0);
            close(connfd);
        }
    }
}

tcp_server::~tcp_server() { close(_sock_fd); }