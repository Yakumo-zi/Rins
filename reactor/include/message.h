#ifndef MESSAGE_H
#define MESSAGE_H
#include <bits/stdint-uintn.h>
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
                          tcp_client *client, void *user_data);
#endif