#include "config_file.h"
#include "echo_message.pb.h"
#include "event_loop.h"
#include "net_connection.h"
#include "tcp_server.h"
#include <bits/stdint-uintn.h>
#include <string.h>
#include <string>
void callback_busi(const char *data, uint32_t len, int msgid,
                   net_connection *conn, void *user_data) {
    qps_test::EchoMessage request, response;
    request.ParseFromArray(data, len);
    response.set_id(request.id());
    response.set_content(request.content());

    std::string response_string;

    response.SerializeToString(&response_string);
    conn->send_message(response_string.c_str(), response_string.size(), msgid);
}

int main() {

    event_loop loop;
    config_file::set_path("./server.conf");

    std::string ip =
        config_file::instance()->get_string("reactor", "ip", "0.0.0.0");
    short port = config_file::instance()->get_number("reactor", "port", 8888);

    printf("ip = %s, port = %d\n", ip.c_str(), port);

    tcp_server server(&loop, ip.c_str(), port);
    server.add_msg_router(1, callback_busi);
    loop.event_process();
}