#include "tcp_conn.h"
#include "event_loop.h"
#include "message.h"
#include "tcp_server.h"
#include <bits/stdint-uintn.h>
#include <cerrno>
#include <cstdio>
#include <ctime>
#include <fcntl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

void callback_busi(const char *data, uint32_t len, int msgid, void *args,
                   tcp_conn *conn) {
    conn->send_message(data, len, msgid);
}

static void conn_rd_callback(event_loop *loop, int fd, void *args) {
    tcp_conn *conn = (tcp_conn *)args;
    conn->do_read();
}

static void conn_wt_callback(event_loop *loop, int fd, void *args) {
    tcp_conn *conn = (tcp_conn *)args;
    conn->do_write();
}

tcp_conn::tcp_conn(int fd, event_loop *loop) {
    _loop = loop;
    _connfd = fd;
    int flag = fcntl(_connfd, F_GETFL, 0);
    fcntl(_connfd, F_SETFL, O_NONBLOCK | flag);

    // 禁止做读写缓存，降低小包延迟
    int op = 1;
    setsockopt(MSG_CONFIRM, IPPROTO_TCP, TCP_NODELAY, &op, sizeof(op));

    _loop->add_io_event(_connfd, conn_rd_callback, EPOLLIN, this);
    if (tcp_server::conn_start_cb) {
        tcp_server::conn_start_cb(this, tcp_server::conn_start_cb_args);
    }
    tcp_server::increase_conn(_connfd, this);
}

void tcp_conn::do_read() {
    int ret = _ibuf.read_data(_connfd);
    if (ret == -1) {
        fprintf(stderr, "tcp_conn::do_read read data from socket\n");
        clean_conn();
        return;
    } else if (ret == 0) {
        printf("tcp_conn::do_read connection closed by peer\n");
        clean_conn();
        return;
    }

    msg_head head;
    while (_ibuf.length() >= MESSAGE_HEAD_LEN) {
        memcpy(&head, _ibuf.data(), MESSAGE_HEAD_LEN);
        if (head.length > MESSAGE_LENGTH_LIMIT || head.length < 0) {
            fprintf(stderr,
                    "tcp_conn::do_read data format error, need close, msglen = "
                    "%d\n",
                    head.length);
            clean_conn();
            break;
        }
        if (_ibuf.length() < MESSAGE_HEAD_LEN + head.length) {
            break;
        }
        _ibuf.pop(MESSAGE_HEAD_LEN);
        printf("tcp_conn::do_read read data: %s\n", _ibuf.data());
        // callback_busi(_ibuf.data(), head.length, head.id, nullptr, this);
        tcp_server::router.call(head.id, head.length, _ibuf.data(), this);
        _ibuf.pop(head.length);
    }
    _ibuf.adjust();
    return;
}

void tcp_conn::do_write() {
    while (_obuf.length()) {
        int ret = _obuf.write2fd(_connfd);
        if (ret == -1) {
            fprintf(stderr, "tcp_conn::do_write write2fd error, close conn!\n");
            printf("errinfo: %s\n", strerror(errno));
            clean_conn();
            return;
        }
        if (ret == 0) {
            break;
        }
    }
    if (_obuf.length() == 0) {
        _loop->del_io_event(_connfd, EPOLLOUT);
    }
}
int tcp_conn::send_message(const char *data, int msglen, int msgid) {
    printf("tcp_conn::send_message server send_message: %s:%d, msgid = %d\n",
           data, msglen, msgid);
    bool active_epollout = false;
    if (_obuf.length() == 0) {
        //如果现在已经数据都发送完了，那么是一定要激活写事件的
        //如果有数据，说明数据还没有完全写完到对端，那么没必要再激活等写完再激活
        active_epollout = true;
    }
    msg_head head;
    head.id = msgid;
    head.length = msglen;

    // 写消息头
    int ret = _obuf.send_data((const char *)&head, MESSAGE_HEAD_LEN);
    if (ret != 0) {
        fprintf(stderr, "tcp_conn::send_message send head error\n");
        return -1;
    }

    // 写消息体
    ret = _obuf.send_data(data, msglen);
    if (ret != 0) {
        // 写消息体失败，回滚将消息头的发送也取消
        _obuf.pop(MESSAGE_HEAD_LEN);
        return -1;
    }

    if (active_epollout == true) {
        _loop->add_io_event(_connfd, conn_wt_callback, EPOLLOUT, this);
    }

    return 0;
}
void tcp_conn::clean_conn() {
    if (tcp_server::conn_close_cb) {
        tcp_server::conn_close_cb(this, tcp_server::conn_close_cb_args);
    }
    tcp_server::decrease_conn(_connfd);
    _loop->del_io_event(_connfd);
    _ibuf.clear();
    _obuf.clear();
    int fd = _connfd;
    _connfd = -1;
    close(fd);
}