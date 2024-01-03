#include "config_file.h"
#include "dns_route.h"
#include "net_connection.h"
#include "rins.pb.h"
#include "rins_reactor.h"
#include "tcp_server.h"
#include <bits/stdint-uintn.h>
#include <cstdio>
#include <mysql/mysql.h>
#include <string>
void get_route(const char *data, uint32_t len, int msgid,
               net_connection *net_conn, void *user_data) {
    rins::GetRouteRequest req;
    req.ParseFromArray(data, len);
    int modid = req.modid(), cmdid = req.cmdid();

    auto hosts = route::instance()->get_hosts(modid, cmdid);

    rins::GetRouteResponse resp;
    resp.set_cmdid(cmdid);
    resp.set_modid(modid);

    for (auto it = hosts.begin(); it != hosts.end(); it++) {
        uint64_t ip_port = *it;
        rins::HostInfo host;
        host.set_ip((uint32_t)(ip_port >> 32));
        host.set_port((int)ip_port);

        resp.add_host()->CopyFrom(host);
    }

    std::string resp_string;
    resp.SerializeToString(&resp_string);
    net_conn->send_message(resp_string.c_str(), resp_string.length(),
                           rins::ID_GetRouteResponse);
}
int main(int argc, char *argv[]) {
    event_loop loop;
    config_file::set_path("../conf/dns.conf");
    std::string ip =
        config_file::instance()->get_string("reactor", "ip", "127.0.0.1");
    short port = config_file::instance()->get_number("reactor", "port", 7778);
    tcp_server *server = new tcp_server(&loop, ip.c_str(), port);

    server->add_msg_router(rins::ID_GetRouteRequest, get_route);

    loop.event_process();
    return 0;
}