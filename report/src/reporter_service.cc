#include "config_file.h"
#include "store_thread.h"
#include "rins.pb.h"
#include "rins_reactor.h"
#include "store_report.h"
#include "thread_queue.h"
thread_queue<rins::ReportStatusRequest> **queues=nullptr;
int thread_cnt = 0;

void get_report_status(const char *data, uint32_t len, int msgid,
                       net_connection *conn, void *user_data) {
    rins::ReportStatusRequest req;

    req.ParseFromArray(data, len);

    store_report sr;
    sr.store(req);

    static int index = 0;
    //将消息发送给某个线程消息队列
    queues[index]->send(req);
    index++;
    index = index % thread_cnt;
}

void create_reportdb_thread() {
    thread_cnt =
        config_file::instance()->get_number("reporter", "db_thread_cnt", 3);
    queues = new thread_queue<rins::ReportStatusRequest> *[thread_cnt];
    if (queues == nullptr) {
        fprintf(stderr,
                "create thread_queue<lars::ReportStatusRequest>*[%d], error",
                thread_cnt);
        exit(1);
    }

    for (int i = 0; i < thread_cnt; i++) {
        //给当前线程创建一个消息队列queue
        queues[i] = new thread_queue<rins::ReportStatusRequest>();
        if (queues == NULL) {
            fprintf(stderr, "create thread_queue error\n");
            exit(1);
        }

        pthread_t tid;
        int ret = pthread_create(&tid, NULL, store_main, queues[i]);
        if (ret == -1) {
            perror("pthread_create");
            exit(1);
        }
        pthread_detach(tid);
    }
}

int main() {
    event_loop loop;

    //加载配置文件
    config_file::set_path("../conf/report.conf");
    std::string ip =
        config_file::instance()->get_string("reactor", "ip", "0.0.0.0");
    short port = config_file::instance()->get_number("reactor", "port", 7779);

    //创建tcp server
    tcp_server server(&loop, ip.c_str(), port);

    //添加数据上报请求处理的消息分发处理业务
    server.add_msg_router(rins::ID_ReportStatusRequest, get_report_status);
    create_reportdb_thread();
    //启动事件监听
    loop.event_process();
    return 0;
}