#include "event_base.h"
#include "tcp_server.h"
int main() {
    event_loop loop;
    tcp_server server(&loop, "0.0.0.0", 8080);
    loop.event_process();
    return 0;
}