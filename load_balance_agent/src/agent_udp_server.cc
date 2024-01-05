#include "event_loop.h"
#include "main_server.h"
#include "rins_reactor.h"
#include "udp_server.h"

void *agent_server_main(void *args) {
    int *index = (int *)args;
    short port = *index + 9999;
    event_loop loop;
    udp_server server(&loop, "0.0.0.0", port);

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