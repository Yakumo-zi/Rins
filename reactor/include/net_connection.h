#ifndef NET_CONNECTION_H
#define NET_CONNECTION_H
class net_connection {
  public:
    virtual int send_message(const char *data, int datalen, int msgid) = 0;
};
#endif