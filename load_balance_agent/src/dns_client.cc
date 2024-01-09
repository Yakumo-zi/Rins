#include "config_file.h"
#include "event_loop.h"
#include "main_server.h"
#include "net_connection.h"
#include "rins.pb.h"
#include "rins_reactor.h"
#include "route_lb.h"
#include "tcp_client.h"
#include <pthread.h>
#include <sys/types.h>

void new_dns_request(event_loop *loop, int fd, void *args) {
    tcp_client *client = (tcp_client *)args;
    std::queue<rins::GetRouteRequest> queue;

    dns_queue->recv(queue);
    while (!queue.empty()) {
        auto msg = queue.front();
        queue.pop();
        std::string request_string;

        msg.SerializeToString(&request_string);
        client->send_message(request_string.c_str(), request_string.length(),
                             rins::ID_GetRouteRequest);
    }
}
static void deal_recv_route(const char *data, u_int32_t len, int msgid,
                            net_connection *conn, void *userdata) {
    rins::GetRouteResponse resp;

    resp.ParseFromArray(data, msgid);

    int modid = resp.modid();
    int cmdid = resp.cmdid();
    int index = (modid + cmdid) % 3;
    r_lb[index]->update_host(modid, cmdid, resp);
}
void *dns_client_thread(void *args) {
    printf("dns client thread start\n");
    event_loop loop;

    std::string ip = config_file::instance()->get_string("dnsserver", "ip", "");
    short port = config_file::instance()->get_number("dnsserver", "port", 0);

    tcp_client client(&loop, ip.c_str(), port, "dns client");

    dns_queue->set_loop(&loop);
    dns_queue->set_callback(new_dns_request, &client);

    client.add_msg_router(rins::ID_GetHostResponse, deal_recv_route);
    loop.event_process();
    return nullptr;
}

void start_dns_client() {
    //开辟一个线程
    pthread_t tid;

    //启动线程业务函数
    int ret = pthread_create(&tid, NULL, dns_client_thread, NULL);
    if (ret == -1) {
        perror("pthread_create");
        exit(1);
    }

    //设置分离模式
    pthread_detach(tid);
}