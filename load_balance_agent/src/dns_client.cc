#include "config_file.h"
#include "event_loop.h"
#include "main_server.h"
#include "rins.pb.h"
#include "rins_reactor.h"
#include "tcp_client.h"
#include <pthread.h>
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
void *dns_client_thread(void *args) {
    printf("dns client thread start\n");
    event_loop loop;

    std::string ip = config_file::instance()->get_string("dnsserver", "ip", "");
    short port = config_file::instance()->get_number("dnsserver", "port", 0);

    tcp_client client(&loop, ip.c_str(), port, "dns client");

    dns_queue->set_loop(&loop);
    dns_queue->set_callback(new_dns_request, &client);

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