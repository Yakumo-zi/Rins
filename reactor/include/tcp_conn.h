#ifndef TCP_CONN_H
#define TCP_CONN_H
#include "event_loop.h"
#include "net_connection.h"
#include "reactor_buf.h"

class tcp_conn : public net_connection {
  public:
    tcp_conn(int connfd, event_loop *loop);
    void do_read();
    void do_write();
    void clean_conn();
    virtual int send_message(const char *data, int msglen, int msgid) override;
    int get_fd() override { return _connfd; }

  private:
    int _connfd;
    event_loop *_loop;
    output_buf _obuf;
    input_buf _ibuf;
};
#endif