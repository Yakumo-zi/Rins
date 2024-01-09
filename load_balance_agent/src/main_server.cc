#include "main_server.h"
#include "config_file.h"
#include "rins.pb.h"
#include "route_lb.h"
#include "thread_queue.h"

thread_queue<rins::ReportStatusRequest> *report_queue = nullptr;
thread_queue<rins::GetRouteRequest> *dns_queue = nullptr;
route_lb *r_lb[3];

static void create_route_lb() {
    for (int i = 0; i < 3; i++) {
        int id = i + 1;
        r_lb[i] = new route_lb(id);
        if (r_lb[i] == nullptr) {
            fprintf(stderr, "no more space to new route_lb\n");
            exit(1);
        }
    }
}
static void init_lb_agent() {
    config_file::set_path("../conf/lb_agent.conf");
    create_route_lb();
}
int main() {
    init_lb_agent();

    start_UDP_servers();
    report_queue = new thread_queue<rins::ReportStatusRequest>();
    if (report_queue == nullptr) {
        fprintf(stderr, "create report queue error!\n");
        exit(1);
    }
    start_report_client();

    dns_queue = new thread_queue<rins::GetRouteRequest>();
    if (dns_queue == nullptr) {
        fprintf(stderr, "create dns queue error!\n");
        exit(1);
    }
    start_dns_client();
    std::cout << "done!" << std::endl;
    while (1) {
        sleep(10);
    }

    return 0;
}