#include "event_loop.h"
#include "main_server.h"
#include "net_connection.h"
#include "rins.pb.h"
#include "rins_reactor.h"
#include "route_lb.h"
#include "udp_server.h"
#include <bits/stdint-uintn.h>

static void get_host_cb(const char *data, uint32_t len, int msgid,
                        net_connection *conn, void *userdata) {
    rins::GetHostRequest req;
    req.ParseFromArray(data, len);

    int modid = req.modid();
    int cmdid = req.cmdid();

    rins::GetHostResponse resp;

    resp.set_seq(req.seq());
    resp.set_cmdid(cmdid);
    resp.set_modid(modid);

    route_lb *ptr_route_lb = (route_lb *)userdata;

    ptr_route_lb->get_host(modid, cmdid, resp);

    std::string resp_string;
    resp.SerializeToString(&resp_string);
    conn->send_message(resp_string.c_str(), resp_string.length(), msgid);
}

void *agent_server_main(void *args) {
    int *index = (int *)args;
    short port = *index + 8888;
    event_loop loop;
    udp_server server(&loop, "0.0.0.0", port);
    server.add_msg_router(rins::ID_GetHostRequest, get_host_cb);

    printf("agent UDP server :port %d is started...\n", port);
    loop.event_process();
    return nullptr;
}

void start_UDP_servers() {
    for (int i = 0; i < 3; i++) {
        pthread_t tid;
        int j = i;
        int ret = pthread_create(&tid, NULL, agent_server_main, &j);
        if (ret == -1) {
            perror("pthread_create");
            exit(1);
        }

        pthread_detach(tid);
    }
}