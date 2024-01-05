#include "main_server.h"
#include "rins.pb.h"
#include "thread_queue.h"

thread_queue<rins::ReportStatusRequest> *report_queue = nullptr;
thread_queue<rins::GetRouteRequest> *dns_queue = nullptr;

int main() {
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