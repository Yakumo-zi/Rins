#include "../../include/tcp_server.h"
int main() {
    tcp_server server("0.0.0.0", 8080);
    server.do_accept();
    return 0;
}