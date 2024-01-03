#ifndef NET_CONNECTION_H
#define NET_CONNECTION_H
class net_connection {
  public:
    net_connection() : param(nullptr) {}
    virtual int send_message(const char *data, int datalen, int msgid) = 0;
    virtual int get_fd() = 0;
    void *param;
};
typedef void (*conn_callback)(net_connection *conn, void *args);
#endif