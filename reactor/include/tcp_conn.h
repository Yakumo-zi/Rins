#ifndef TCP_CONN_H
#define TCP_CONN_H
#include "event_loop.h"
#include "reactor_buf.h"
#include "net_connection.h"

class tcp_conn:public net_connection {
  public:
    tcp_conn(int connfd, event_loop *loop);
    void do_read();
    void do_write();
    void clean_conn();
    int send_message(const char *data, int msglen, int msgid) override;

  private:
    int _connfd;
    event_loop *_loop;
    output_buf _obuf;
    input_buf _ibuf;
};
#endif