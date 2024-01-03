
#include "echo_message.pb.h"
#include "event_loop.h"
#include "message.h"
#include "net_connection.h"
#include "tcp_client.h"
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <pthread.h>

struct Qps {
    Qps() {
        last_time = time(NULL);
        succ_cnt = 0;
    }

    long last_time; //最后一次发包时间 ms为单位
    int succ_cnt;   //成功收到服务器回显的次数
};
void busi(const char *data, uint32_t len, int msgid, net_connection *conn,
          void *user_data) {

    Qps *qps = (Qps *)user_data;
    qps_test::EchoMessage request, response;
    if (response.ParseFromArray(data, len) == false) {
        printf("server call back data error\n");
        return;
    }
    if (response.content() == "Hello Rins!!!") {
        qps->succ_cnt++;
    }
    long current_time = time(nullptr);
    if (current_time - qps->last_time >= 1) {
        printf("---> qps = %d <----\n", qps->succ_cnt);
        qps->last_time = current_time;
        qps->succ_cnt = 0;
    }
    //给服务端发送新的请求
    request.set_id(response.id() + 1);
    request.set_content(response.content());

    std::string requestString;
    request.SerializeToString(&requestString);

    conn->send_message(requestString.c_str(), requestString.size(), msgid);
}
void connection_start(net_connection *client, void *args) {
    qps_test::EchoMessage request;
    request.set_id(1);
    request.set_content("Hello Rins!!!");
    std::string request_string;
    request.SerializeToString(&request_string);
    int msgid = 1;
    client->send_message(request_string.c_str(), request_string.size(), msgid);
}
void *thread_main(void *arg) {
    event_loop loop;
    tcp_client client(&loop, "127.0.0.1", 7777, "qps client");
    Qps qps;
    //设置回调
    client.add_msg_router(1, busi, (void *)&qps);

    //设置链接创建成功之后Hook
    client.set_conn_start(connection_start, nullptr);

    loop.event_process();

    return NULL;
}
int main(int argc, char *argv[]) {
    if (argc == 1) {
        printf("Usage: ./client [threadNum]\n");
        return 1;
    }
    int thread_num = atoi(argv[1]);
    pthread_t *tids;
    tids = new pthread_t[thread_num];
    for (int i = 0; i < thread_num; i++) {
        pthread_create(&tids[i], NULL, thread_main, NULL);
    }

    for (int i = 0; i < thread_num; i++) {
        pthread_join(tids[i], NULL);
    }

    return 0;
}