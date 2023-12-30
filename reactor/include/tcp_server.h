#ifndef TCP_SERVER_H
#define TCP_SERVER_H

#include "event_base.h"
#include "event_loop.h"
#include <bits/stdint-uintn.h>
#include <netinet/in.h>
#include <sys/socket.h>
class tcp_server {
  public:
    tcp_server(event_loop* loop,const char *ip, uint16_t port);
    void do_accept();
    ~tcp_server();

  private:
    int _sock_fd;
    sockaddr_in _in_connaddr;
    socklen_t _addrlen;
    event_loop* _event_loop;
};

#endif