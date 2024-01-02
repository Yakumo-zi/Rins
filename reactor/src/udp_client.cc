#include "udp_client.h"
#include "event_loop.h"
#include <arpa/inet.h>
#include <bits/stdint-uintn.h>
#include <cstring>
#include <netinet/in.h>
#include <strings.h>
#include <unistd.h>

void read_callback(event_loop *loop, int fd, void *args) {
    udp_client *cli = (udp_client *)args;
    cli->do_read();
}

udp_client::udp_client(event_loop *loop, const char *ip, uint16_t port) {
    _sockfd =
        socket(AF_INET, SOCK_DGRAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_UDP);
    if (_sockfd == -1) {
        perror("create socket error");
        exit(1);
    }

    struct sockaddr_in servaddr;
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    inet_aton(ip, &servaddr.sin_addr);
    servaddr.sin_port = htons(port);

    // 2 链接
    int ret = connect(_sockfd, (const sockaddr *)&servaddr, sizeof(servaddr));
    if (ret == -1) {
        perror("connect");
        exit(1);
    }

    // 3 添加读事件
    _loop = loop;
    _loop->add_io_event(_sockfd, read_callback, EPOLLIN, this);
}

udp_client::~udp_client() {
    _loop->del_io_event(_sockfd);
    close(_sockfd);
}

void udp_client::do_read() {
    while (true) {
        int pkt_len =
            recvfrom(_sockfd, _read_buf, sizeof(_read_buf), 0, NULL, NULL);
        if (pkt_len == -1) {
            if (errno == EINTR) {
                continue;
            } else if (errno == EAGAIN) {
                break;
            } else {
                perror("recvfrom()");
                break;
            }
        }

        //处理客户端包
        msg_head head;
        memcpy(&head, _read_buf, MESSAGE_HEAD_LEN);
        if (head.length > MESSAGE_LENGTH_LIMIT || head.length < 0 ||
            head.length + MESSAGE_HEAD_LEN != pkt_len) {
            //报文格式有问题
            fprintf(
                stderr,
                "do_read, data error, msgid = %d, msglen = %d, pkt_len = %d\n",
                head.id, head.length, pkt_len);
            continue;
        }

        //调用注册的路由业务
        _router.call(head.id, head.id, _read_buf + MESSAGE_HEAD_LEN, this);
    }
}
void udp_client::add_msg_router(int msgid, msg_callback *cb, void *user_data) {
    _router.register_msg_router(msgid, cb, user_data);
}

int udp_client::send_message(const char *data, int msglen, int msgid) {
    if (msglen > MESSAGE_LENGTH_LIMIT) {
        fprintf(stderr, "too large message to send\n");
        return -1;
    }

    msg_head head;
    head.length = msglen;
    head.id = msgid;

    memcpy(_write_buf, &head, MESSAGE_HEAD_LEN);
    memcpy(_write_buf + MESSAGE_HEAD_LEN, data, msglen);

    int ret =
        sendto(_sockfd, _write_buf, msglen + MESSAGE_HEAD_LEN, 0, NULL, 0);
    if (ret == -1) {
        perror("sendto()..");
        return -1;
    }

    return ret;
}
