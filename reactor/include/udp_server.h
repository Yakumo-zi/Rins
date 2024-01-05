#ifndef UDP_SERVER_H
#define UDP_SERVER_H

#include "event_loop.h"
#include "message.h"
#include "net_connection.h"
#include <bits/stdint-uintn.h>
#include <netinet/in.h>
#include <unistd.h>
class udp_server : public net_connection {
  public:
    udp_server(event_loop *loop, const char *ip, uint16_t port);
    virtual int send_message(const char *data, int msglen, int msglid) override;
    void add_msg_router(int msgid, msg_callback *cb, void *user_data = nullptr);
    int get_fd() override{
      return _sockfd;
    }
    ~udp_server();
    void do_read();

  private:
    int _sockfd;
    char _read_buf[MESSAGE_LENGTH_LIMIT];
    char _write_buf[MESSAGE_LENGTH_LIMIT];
    event_loop *_loop;
    sockaddr_in _client_addr;
    socklen_t _client_addrlen;

    msg_router _router;
};
#endif