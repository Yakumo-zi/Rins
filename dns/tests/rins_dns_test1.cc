#include "event_loop.h"
#include "net_connection.h"
#include "rins.pb.h"
#include "rins_reactor.h"
#include "tcp_client.h"
#include <bits/stdint-uintn.h>
#include <cstdlib>
#include <cstring>
#include <string>
#include <unistd.h>
struct Option {
    Option() : ip(nullptr), port(0) {}
    char *ip;
    short port;
};
Option option;
void Usage() { printf("Usage: ./test -h ip -p port\n"); }

void parse_option(int argc, char *argv[]) {

    for (int i = 0; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0) {
            option.ip = argv[i + 1];
        } else if (strcmp(argv[i], "-p") == 0) {
            option.port = atoi(argv[i + 1]);
        }
    }
    if (!option.ip || !option.port) {
        Usage();
        exit(1);
    }
}
void on_connection(net_connection *conn, void *args) {
    rins::GetRouteRequest req;
    req.set_modid(1);
    req.set_cmdid(2);
    std::string request_string;
    req.SerializeToString(&request_string);
    conn->send_message(request_string.c_str(), request_string.length(),
                       rins::ID_GetRouteRequest);
}

void deal_get_route(const char *data, uint32_t len, int msgid,
                    net_connection *conn, void *user_data) {
    rins::GetRouteResponse resp;
    resp.ParseFromArray(data, len);
    //打印数据
    printf("modid = %d\n", resp.modid());
    printf("cmdid = %d\n", resp.cmdid());
    printf("host_size = %d\n", resp.host_size());

    for (int i = 0; i < resp.host_size(); i++) {
        printf("-->ip = %u\n", resp.host(i).ip());
        printf("-->port = %d\n", resp.host(i).port());
    }

    rins::GetRouteRequest req;
    req.set_cmdid(resp.cmdid());
    req.set_modid(resp.modid());
    std::string req_string;
    req.SerializeToString(&req_string);
    conn->send_message(req_string.c_str(), req_string.length(),
                       rins::ID_GetRouteRequest);
}
int main(int argc, char *argv[]) {
    parse_option(argc, argv);
    event_loop loop;
    tcp_client *client;
    client = new tcp_client(&loop, option.ip, option.port, "rins_dns_test");
    if (client == nullptr) {
        fprintf(stderr, "client == NULL\n");
        exit(1);
    } //客户端成功建立连接，首先发送请求包
    client->set_conn_start(on_connection, nullptr);

    //设置服务端回应包处理业务
    client->add_msg_router(rins::ID_GetRouteResponse, deal_get_route);

    loop.event_process();
    return 0;
}