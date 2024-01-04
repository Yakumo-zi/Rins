#include "dns_route.h"
#include "event_loop.h"
#include "net_connection.h"
#include "rins.pb.h"
#include "rins_reactor.h"
#include "subscribe.h"
#include "tcp_server.h"
#include <bits/stdint-uintn.h>
#include <cstdio>
#include <unordered_set>

tcp_server *server;

void get_route(const char *data, uint32_t len, int msgid,
               net_connection *net_conn, void *user_data) {
    // 1. 解析proto文件
    rins::GetRouteRequest req;

    req.ParseFromArray(data, len);

    // 2. 得到modid 和 cmdid
    int modid, cmdid;

    modid = req.modid();
    cmdid = req.cmdid();
    uint64_t mod = ((uint64_t)modid << 32) + cmdid;
    std::unordered_set<uint64_t> *sub_list =
        (std::unordered_set<uint64_t> *)net_conn->param;
    if (sub_list == nullptr) {
        printf("sub_list == nullptr;\n");
    }
    if (sub_list->find(mod) == sub_list->end()) {
        sub_list->insert(mod);
        subscribe_list::instance()->subscribe(mod, net_conn->get_fd());
    }

    // 3. 根据modid/cmdid 获取 host信息
    auto hosts = route::instance()->get_hosts(modid, cmdid);

    // 4. 将数据打包成protobuf
    rins::GetRouteResponse rsp;

    rsp.set_modid(modid);
    rsp.set_cmdid(cmdid);

    for (auto it = hosts.begin(); it != hosts.end(); it++) {
        uint64_t ip_port = *it;
        rins::HostInfo host;
        host.set_ip((uint32_t)(ip_port >> 32));
        host.set_port((int)(ip_port));
        rsp.add_host()->CopyFrom(host);
    }

    // 5. 发送给客户端
    std::string responseString;
    rsp.SerializeToString(&responseString);
    net_conn->send_message(responseString.c_str(), responseString.size(),
                           rins::ID_GetRouteResponse);
}

void create_subscribe(net_connection *conn, void *args) {
    conn->param = new std::unordered_set<uint64_t>;
}

void clear_subscribe(net_connection *conn, void *args) {
    auto sub_list = (std::unordered_set<uint64_t> *)conn->param;
    for (auto it = sub_list->begin(); it != sub_list->end(); it++) {
        uint64_t mod = *it;
        subscribe_list::instance()->unsubscribe(mod, conn->get_fd());
    }
    delete sub_list;
    conn->param = nullptr;
}

void *check_route_changes(void *args) {
    int wait_time = 10; // 10s自动修改一次，也可以从配置文件读取
    long last_load_time = time(NULL);

    //清空全部的RouteChange
    route::instance()->remove_changes(true);

    // 1 判断是否有修改
    while (true) {
        sleep(1);
        long current_time = time(NULL);

        // 1.1 加载RouteVersion得到当前版本号
        int ret = route::instance()->load_version();
        if (ret == 1) {
            // version改版 有modid/cmdid修改
            // 2 如果有修改

            // 2.1 将最新的RouteData加载到_temp_pointer中
            if (route::instance()->load_route_data() == 0) {
                // 2.2 更新_temp_pointer数据到_data_pointer map中
                route::instance()->swap();
                last_load_time = current_time; //更新最后加载时间
            }

            // 2.3 获取被修改的modid/cmdid对应的订阅客户端,进行推送
            std::vector<uint64_t> changes;
            route::instance()->load_changes(changes);

            //推送
            subscribe_list::instance()->publish(changes);

            // 2.4 删除当前版本之前的修改记录
            route::instance()->remove_changes(false);

        } else {
            // 3 如果没有修改
            if (current_time - last_load_time >= wait_time) {
                // 3.1 超时,加载最新的temp_pointer
                if (route::instance()->load_route_data() == 0) {
                    // 3.2 _temp_pointer数据更新到_data_pointer map中
                    route::instance()->swap();
                    last_load_time = current_time;
                }
            }
        }
    }

    return nullptr;
}

int main() {
    event_loop loop;
    config_file::set_path("../conf/dns.conf");
    std::string ip =
        config_file::instance()->get_string("reactor", "ip", "0.0.0.0");
    short port = config_file::instance()->get_number("reactor", "port", 7778);

    server = new tcp_server(&loop, ip.c_str(), port);

    server->set_conn_start(create_subscribe, nullptr);
    server->set_conn_close(clear_subscribe, nullptr);

    server->add_msg_router(rins::ID_GetRouteRequest, get_route);
    //开辟backend thread 周期性检查db数据库route信息的更新状态
    pthread_t tid;
    int ret = pthread_create(&tid, NULL, check_route_changes, NULL);
    if (ret == -1) {
        perror("pthread_create backendThread");
        exit(1);
    }
    //设置分离模式
    pthread_detach(tid);
    // =================================================
    printf("lars dns service ....\n");
    loop.event_process();

    return 0;
}