#include "udp_server.h"
#include "event_loop.h"
#include "message.h"
#include <arpa/inet.h>
#include <asm-generic/errno-base.h>
#include <bits/stdint-uintn.h>
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <netinet/in.h>
#include <signal.h>
#include <strings.h>
#include <sys/socket.h>

static void read_callback(event_loop *loop, int fd, void *args) {
    udp_server *server = (udp_server *)args;
    server->do_read();
}
void udp_server::do_read() {
    while (true) {
        int pkg_len = recvfrom(_sockfd, _read_buf, sizeof(_read_buf), 0,
                               (sockaddr *)&_client_addr, &_client_addrlen);

        if (pkg_len == -1) {
            if (errno == EINTR) {
                continue;
            } else if (errno == EAGAIN) {
                break;
            } else {
                perror("recvfrom\n");
                break;
            }
        }
        msg_head head;
        memcpy(&head, _read_buf, MESSAGE_HEAD_LEN);
        if (head.length > MESSAGE_LENGTH_LIMIT || head.length < 0 ||
            head.length + MESSAGE_HEAD_LEN != pkg_len) {
            //报文格式有问题
            fprintf(stderr,
                    "udp_server::do_read, data error, msgid = %d, msglen = %d, "
                    "pkg_len = %d\n",
                    head.id, head.length, pkg_len);
            continue;
        }
        _router.call(head.id, head.length, _read_buf+MESSAGE_HEAD_LEN, this);
    }
}
udp_server::udp_server(event_loop *loop, const char *ip, uint16_t port) {
    // 1 忽略一些信号
    if (signal(SIGHUP, SIG_IGN) == SIG_ERR) {
        perror("signal ignore SIGHUB");
        exit(1);
    }
    if (signal(SIGPIPE, SIG_IGN) == SIG_ERR) {
        perror("signal ignore SIGPIPE");
        exit(1);
    }
    // SOCK_CLOEXEC在execl中使用该socket则关闭，在fork中使用该socket不关闭
    _sockfd =
        socket(AF_INET, SOCK_DGRAM | SOCK_CLOEXEC | SOCK_NONBLOCK, IPPROTO_UDP);
    if (_sockfd == -1) {
        perror("socket create ");
        exit(1);
    }
    struct sockaddr_in servaddr;
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    inet_aton(ip, &servaddr.sin_addr); //设置ip
    servaddr.sin_port = htons(port);   //设置端口

    // 4 绑定
    if (bind(_sockfd, (const sockaddr *)&servaddr, sizeof(servaddr)) == -1) {
        perror("socket bind ");
        exit(1);
    }

    _loop = loop;
    bzero(&_client_addr, sizeof(_client_addr));
    _client_addrlen = sizeof(_client_addr);

    printf("server on %s:%u is running...\n", ip, port);

    _loop->add_io_event(_sockfd, read_callback, EPOLLIN, this);
}

int udp_server::send_message(const char *data, int msglen, int msgid) {
    if (msglen > MESSAGE_LENGTH_LIMIT) {
        fprintf(stderr, "too large message to send\n");
        return -1;
    }
    msg_head head;
    head.length = msglen;
    head.id = msgid;

    memcpy(_write_buf, &head, MESSAGE_HEAD_LEN);
    memcpy(_write_buf + MESSAGE_HEAD_LEN, data, msglen);

    int ret = sendto(_sockfd, _write_buf, msglen + MESSAGE_HEAD_LEN, 0,
                     (sockaddr *)&_client_addr, _client_addrlen);
    if (ret == -1) {
        perror("sendto()..");
        return -1;
    }
    return ret;
}

void udp_server::add_msg_router(int msgid, msg_callback *cb, void *user_data) {
    _router.register_msg_router(msgid, cb, user_data);
}

udp_server::~udp_server() {
    _loop->del_io_event(_sockfd);
    close(_sockfd);
}