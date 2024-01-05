#include "event_loop.h"
#include "main_server.h"
#include "rins.pb.h"
#include "rins_reactor.h"
#include "tcp_client.h"
#include "thread_queue.h"
#include <pthread.h>
void new_report_request(event_loop *loop, int fd, void *args) {
    tcp_client *client = (tcp_client *)args;
    std::queue<rins::ReportStatusRequest> queue;

    report_queue->recv(queue);
    while (!queue.empty()) {
        auto msg = queue.front();
        queue.pop();
        std::string request_string;
        msg.SerializeToString(&request_string);
        client->send_message(request_string.c_str(), request_string.length(),
                             rins::ID_ReportStatusRequest);
    }
}
void *report_client_thread(void *args) {
    printf("report client thread start\n");
    event_loop loop;
    // 1 加载配置文件得到repoter ip + port
    std::string ip = config_file::instance()->get_string("reporter", "ip", "");
    short port = config_file::instance()->get_number("reporter", "port", 0);

    // 2 创建客户端
    tcp_client client(&loop, ip.c_str(), port, "reporter client");

    // 3 将 thread_queue消息回调事件，绑定到loop中
    report_queue->set_loop(&loop);
    report_queue->set_callback(new_report_request, &client);

    // 4 启动事件监听
    loop.event_process();
    return nullptr;
}

void start_report_client() {
    //开辟一个线程
    pthread_t tid;

    //启动线程业务函数
    int ret = pthread_create(&tid, NULL, report_client_thread, NULL);
    if (ret == -1) {
        perror("pthread_create");
        exit(1);
    }

    //设置分离模式
    pthread_detach(tid);
}