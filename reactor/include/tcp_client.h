#ifndef TCP_CLIENT_H
#define TCP_CLIENT_H
#include "event_loop.h"
#include "io_buf.h"
#include "message.h"

#include "net_connection.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
class tcp_client : public net_connection {
  public:
    tcp_client(event_loop *loop, const char *ip, unsigned short port,
               const char *name);
    int send_message(const char *data, int msglen, int msgid) override;
    void do_connect();
    int do_read();
    int do_write();
    void clean_conn();
    ~tcp_client();

    void add_msg_router(int msgid, msg_callback *cb,
                        void *user_data = nullptr) {
        _router.register_msg_router(msgid, cb, user_data);
    }
    void set_conn_start(conn_callback cb, void *args) {
        _conn_start_cb = cb;
        _conn_start_cb_args = args;
    }
    void set_conn_close(conn_callback cb, void *args) {
        _conn_close_cb = cb;
        _conn_close_cb_args = args;
    }

  public:
    io_buf _ibuf;
    io_buf _obuf;
    bool connected; //链接是否创建成功
    // server端地址
    sockaddr_in _server_addr;
    conn_callback _conn_start_cb;
    void *_conn_start_cb_args;
    conn_callback _conn_close_cb;
    void *_conn_close_cb_args;

  private:
    int _sockfd;
    socklen_t _addrlen;
    event_loop *_loop;
    const char *_name;
    msg_router _router;
};
#endif
