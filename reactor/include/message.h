#ifndef MESSAGE_H
#define MESSAGE_H
#include "net_connection.h"
#include <bits/stdint-uintn.h>
#include <unordered_map>
struct msg_head {
    int id;
    int length;
};

// 消息头的二进制长度
#define MESSAGE_HEAD_LEN 8

// 消息头+消息体的最大长度限制
#define MESSAGE_LENGTH_LIMIT (65535 - MESSAGE_HEAD_LEN)
class tcp_client;
typedef void msg_callback(const char *data, uint32_t len, int msgid,
                          net_connection *client, void *user_data);

class msg_router {
  public:
    msg_router(){};
    int register_msg_router(int msgid, msg_callback *msg_cb, void *user_data) {
        if (_router.find(msgid) != _router.end()) {
            //该msgID的回调业务已经存在
            return -1;
        }
        _router[msgid] = msg_cb;
        _args[msgid] = user_data;
        return 0;
    }

    void call(int msgid, uint32_t msglen, const char *data,
              net_connection *conn) {
        if (_router.find(msgid) == _router.end()) {
            fprintf(stderr, "msg_router::call msgid %d is not register!\n",
                    msgid);
            return;
        }
        msg_callback *callback = _router[msgid];
        void *user_data = _args[msgid];
        callback(data, msglen, msgid, conn, user_data);
    }

  private:
    std::unordered_map<int, msg_callback *> _router;
    std::unordered_map<int, void *> _args;
};

#endif
