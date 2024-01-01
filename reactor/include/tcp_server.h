#ifndef TCP_SERVER_H
#define TCP_SERVER_H

#include "event_base.h"
#include "event_loop.h"
#include "message.h"
#include "net_connection.h"
#include "tcp_conn.h"
#include <bits/stdint-uintn.h>
#include <netinet/in.h>
#include <pthread.h>
#include <sys/socket.h>
class tcp_server {
  public:
    tcp_server(event_loop *loop, const char *ip, uint16_t port);
    void do_accept();
    void add_msg_router(int msgid, msg_callback *cb,
                        void *user_data = nullptr) {
        router.register_msg_router(msgid, cb, user_data);
    }
    ~tcp_server();

  private:
    int _sock_fd;
    sockaddr_in _in_connaddr;
    socklen_t _addrlen;
    event_loop *_event_loop;

  public:
    static void increase_conn(int connfd, tcp_conn *conn); //新增一个新建的连接
    static void decrease_conn(int connfd);    //减少一个断开的连接
    static void get_conn_num(int *curr_conn); //得到当前链接的刻度
    static tcp_conn **conns; //全部已经在线的连接信息
    static msg_router router;

    static void set_conn_start(conn_callback cb, void *args) {
        conn_start_cb = cb;
        conn_start_cb_args = args;
    }

    static void set_conn_close(conn_callback cb, void *args) {
        conn_close_cb = cb;
        conn_close_cb_args = args;
    }

    static conn_callback conn_start_cb;
    static void *conn_start_cb_args;
    static conn_callback conn_close_cb;
    static void *conn_close_cb_args;

  private:
#define MAX_CONNS 10000
    static int _max_conns;
    static int _cur_conns;
    static pthread_mutex_t _conns_mutex;
};

#endif