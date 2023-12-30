#ifndef TCP_CLIENT_H
#define TCP_CLIENT_H
#include "event_loop.h"
#include "io_buf.h"
#include "message.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
class tcp_client {
  public:
    tcp_client(event_loop *loop, const char *ip, unsigned short port,
               const char *name);
    int send_message(const char *data, int msglen, int msgid);
    void do_connect();
    int do_read();
    int do_write();
    void clean_conn();
    ~tcp_client();

    void set_msg_callback(msg_callback *msg_cb) {
        this->_msg_callback = msg_cb;
    }

  public:
    io_buf _ibuf;
    io_buf _obuf;
    bool connected; //链接是否创建成功
    // server端地址
    sockaddr_in _server_addr;

  private:
    int _sockfd;
    socklen_t _addrlen;
    event_loop *_loop;
    const char *_name;
    msg_callback *_msg_callback;
};
#endif
