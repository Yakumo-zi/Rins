#include "tcp_client.h"
#include "event_loop.h"
#include "message.h"
#include <arpa/inet.h>
#include <asm-generic/errno-base.h>
#include <asm-generic/errno.h>
#include <asm-generic/ioctl.h>
#include <asm-generic/ioctls.h>
#include <asm-generic/socket.h>
#include <assert.h>
#include <bits/types/struct_timeval.h>
#include <cerrno>
#include <cstring>
#include <netinet/in.h>
#include <strings.h>
#include <sys/epoll.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>

// 判断文件描述符是否可写
int check_connect(int socket) {
    fd_set rset;
    FD_ZERO(&rset);
    FD_SET(socket, &rset);

    timeval tm;
    tm.tv_sec = 0;
    tm.tv_usec = 0;
    if (select(socket + 1, nullptr, &rset, nullptr, &tm) <= 0) {
        ::close(socket);
        return -1;
    }
    if (FD_ISSET(socket, &rset)) {
        int err = -1;
        socklen_t len = sizeof(int);
        if (getsockopt(socket, SOL_SOCKET, SO_ERROR, &err, &len) < 0) {
            close(socket);
            printf("tcp_client::check_connect errno:%d %s\n", errno,
                   strerror(errno));
            return -2;
        }
        if (err) {
            errno = err;
            close(socket);
            return -3;
        }
    }
    return 0;
}
static void write_callback(event_loop *loop, int fd, void *args) {
    tcp_client *cli = (tcp_client *)args;
    cli->do_write();
}

static void read_callback(event_loop *loop, int fd, void *args) {
    tcp_client *cli = (tcp_client *)args;
    cli->do_read();
}
//判断链接是否是创建链接，主要是针对非阻塞socket 返回EINPROGRESS错误
static void connection_delay(event_loop *loop, int fd, void *args) {
    tcp_client *cli = (tcp_client *)args;
    loop->del_io_event(fd);
    int result = 0;
    socklen_t result_len = sizeof(result);
    // 判断socket是否出错，如果没出错则连接成功
    getsockopt(fd, SOL_SOCKET, SO_ERROR, &result, &result_len);
    if (result == 0) {
        cli->connected = true;
        if (cli->_conn_start_cb) {
            cli->_conn_start_cb(cli, cli->_conn_start_cb_args);
        }
        printf("tcp_client::connection_delay connect %s:%d succ!\n",
               inet_ntoa(cli->_server_addr.sin_addr),
               ntohs(cli->_server_addr.sin_port));

        const char *msg = "hello rins";
        int msgid = 1;
        cli->send_message(msg, strlen(msg), msgid);
        const char *msg2 = "hello tohsaka!";
        msgid = 2;
        cli->send_message(msg2, strlen(msg2), msgid);

        loop->add_io_event(fd, read_callback, EPOLLIN, cli);
        if (cli->_obuf.length != 0) {
            loop->add_io_event(fd, write_callback, EPOLLOUT, cli);
        }
    } else {
        fprintf(stderr, "tcp_client::connection_delay connection %s:%d error\n",
                inet_ntoa(cli->_server_addr.sin_addr),
                ntohs(cli->_server_addr.sin_port));
    }
}
tcp_client::tcp_client(event_loop *loop, const char *ip, unsigned short port,
                       const char *name)
    : _ibuf(4194304), _obuf(4194304) {
    _sockfd = 0;
    _name = name;
    _loop = loop;
    connected = false;
    bzero(&_server_addr, sizeof(_server_addr));
    inet_aton(ip, &_server_addr.sin_addr);
    _server_addr.sin_family = AF_INET;
    _server_addr.sin_port = htons(port);
    _addrlen = sizeof(_server_addr);
    this->do_connect();
}

void tcp_client::do_connect() {
    if (_sockfd == -1) {
        return;
    }

    _sockfd = socket(AF_INET, SOCK_STREAM | SOCK_CLOEXEC | SOCK_NONBLOCK,
                     IPPROTO_TCP);
    if (_sockfd == -1) {
        fprintf(stderr,
                "tcp_client::do_connect create tcp client socket error\n");
        exit(1);
    }
    int ret = connect(_sockfd, (sockaddr *)&_server_addr, _addrlen);
    if (ret != 0) {
        // fd 是非阻塞 可能会出现这个错误 不代表连接失败 如果 fd
        // 状态是可写那么链接是创建成功的
        if (errno == EINPROGRESS) {
            fprintf(stderr, "tcp_client::do_connect EINPROGRESS\n");
            _loop->add_io_event(_sockfd, connection_delay, EPOLLOUT, this);
        } else {
            fprintf(stderr, "tcp_client::do_connect connection error\n");
            exit(1);
        }
    } else {
        connected = true;
        if (_conn_start_cb) {
            _conn_start_cb(this, _conn_start_cb_args);
        }
        //注册读回调
        _loop->add_io_event(_sockfd, read_callback, EPOLLIN, this);
        //如果写缓冲去有数据，那么也需要触发写回调
        if (this->_obuf.length != 0) {
            _loop->add_io_event(_sockfd, write_callback, EPOLLOUT, this);
        }
        printf("tcp_client::do_connect connect %s:%d succ!\n",
               inet_ntoa(_server_addr.sin_addr), ntohs(_server_addr.sin_port));
    }
}

int tcp_client::do_read() {
    assert(connected == true);
    int need_read = 0;
    if (ioctl(_sockfd, FIONREAD, &need_read) == -1) {
        fprintf(stderr, "tcp_client::do_read ioctl FIONREAD error");
        return -1;
    }
    assert(need_read <= _ibuf.capacity - _ibuf.length);

    int ret;
    do {
        ret = read(_sockfd, _ibuf.data + _ibuf.length, need_read);
    } while (ret == -1 && errno == EINTR);

    if (ret == 0) {
        if (_name != nullptr) {
            printf("tcp_client::do_read %s client: connection close by peer!\n",
                   _name);
        } else {
            printf("tcp_client::do_read client: connection close by peer!\n");
        }
        clean_conn();
        return -1;
    } else if (ret == -1) {
        fprintf(stderr, "tcp_client::do_read() , error\n");
        clean_conn();
        return -1;
    }

    // 读完了
    assert(ret == need_read);

    _ibuf.length += ret;
    msg_head head;
    int msgid, length;
    while (_ibuf.length >= MESSAGE_HEAD_LEN) {
        memcpy(&head, _ibuf.data + _ibuf.head, MESSAGE_HEAD_LEN);
        msgid = head.id;
        length = head.length;
        _ibuf.pop(MESSAGE_HEAD_LEN);
        _router.call(msgid, length, _ibuf.data + _ibuf.head, this);
        _ibuf.pop(length);
    }
    _ibuf.adjust();
    return 0;
}

int tcp_client::do_write() {
    assert(_obuf.head == 0 && _obuf.length);
    int ret;
    while (_obuf.length) {
        do {
            ret = write(_sockfd, _obuf.data + _obuf.head, _obuf.length);
        } while (ret == -1 && errno == EINTR);
        if (ret > 0) {
            _obuf.pop(ret);
            _obuf.adjust();

        } else if (ret == -1 && errno != EAGAIN) {
            fprintf(stderr, "tcp_client::do_write error \n");
            clean_conn();
        } else {
            break;
        }
    }
    if (_obuf.length == 0) {
        // printf("tcp_client::do_write over, del EPOLLOUT\n");
        _loop->del_io_event(_sockfd, EPOLLOUT);
    }
    return 0;
}
//释放链接资源,重置连接
void tcp_client::clean_conn() {
    if (_sockfd != -1) {
        printf("tcp_client::clean_conn, del socket!\n");
        _loop->del_io_event(_sockfd);
        close(_sockfd);
    }

    connected = false;
    if (_conn_close_cb) {
        _conn_close_cb(this, _conn_close_cb_args);
    }
    //重新连接
    this->do_connect();
}
tcp_client::~tcp_client() { close(_sockfd); }

int tcp_client::send_message(const char *data, int msglen, int msgid) {
    if (connected == false) {
        fprintf(stderr,
                "tcp_client::send_message no connected , send message stop!\n");
        return -1;
    }
    bool need_add_event = (_obuf.length == 0) ? true : false;
    if (msglen + MESSAGE_HEAD_LEN > this->_obuf.capacity - _obuf.length) {
        fprintf(stderr,
                "tcp_client::send_message No more space to Write socket!\n");
        return -1;
    }
    msg_head head;
    head.id = msgid;
    head.length = msglen;

    memcpy(_obuf.data + _obuf.length, &head, MESSAGE_HEAD_LEN);
    _obuf.length += MESSAGE_HEAD_LEN;

    memcpy(_obuf.data + _obuf.length, data, msglen);
    _obuf.length += msglen;

    if (need_add_event) {
        _loop->add_io_event(_sockfd, write_callback, EPOLLOUT, this);
    }
    return 0;
}
