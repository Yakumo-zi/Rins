#include "subscribe.h"
#include "dns_route.h"
#include "net_connection.h"
#include "rins.pb.h"
#include "tcp_server.h"
#include <bits/stdint-uintn.h>
#include <pthread.h>
#include <unordered_map>
#include <unordered_set>
#include <vector>

extern tcp_server *server;

subscribe_list *subscribe_list::_instance = nullptr;
pthread_once_t subscribe_list::_once = PTHREAD_ONCE_INIT;

subscribe_list::subscribe_list() {}

void subscribe_list::subscribe(uint64_t mod, int fd) {
    pthread_mutex_lock(&_book_list_lock);
    _book_list[mod].insert(fd);
    pthread_mutex_unlock(&_book_list_lock);
}

void subscribe_list::unsubscribe(uint64_t mod, int fd) {
    pthread_mutex_lock(&_book_list_lock);
    if (_book_list.find(mod) != _book_list.end()) {
        _book_list[mod].erase(fd);
        if (_book_list[mod].empty() == true) {
            _book_list.erase(mod);
        }
    }
    pthread_mutex_unlock(&_book_list_lock);
}

void subscribe_list::make_publish_map(
    std::unordered_set<int> &online_fds,
    std::unordered_map<int, std::unordered_set<uint64_t>> &need_publish) {
    pthread_mutex_lock(&_push_list_lock);
    for (auto it = _push_list.begin(); it != _push_list.end(); it++) {
        if (online_fds.find(it->first) != online_fds.end()) {
            need_publish[it->first] = _push_list[it->first];
            _push_list.erase(it);
        }
    }
    pthread_mutex_unlock(&_push_list_lock);
}
static void push_change_task(event_loop *loop, void *args) {
    subscribe_list *subscribe = (subscribe_list *)args;
    std::unordered_set<int> online_fds;
    loop->get_listen_fds(online_fds);
    std::unordered_map<int, std::unordered_set<uint64_t>> need_publish;
    subscribe->make_publish_map(online_fds, need_publish);

    for (auto &it : need_publish) {
        int fd = it.first;
        for (auto st : it.second) {
            int modid = int((st) >> 32);
            int cmdid = st;
            rins::GetRouteResponse resp;
            resp.set_modid(modid);
            resp.set_cmdid(cmdid);

            auto hosts = route::instance()->get_hosts(modid, cmdid);

            for (auto &hit : hosts) {
                uint64_t ip_port_pair = hit;
                rins::HostInfo host_info;
                host_info.set_ip((uint32_t)(ip_port_pair >> 32));
                host_info.set_port((int)ip_port_pair);
                resp.add_host()->CopyFrom(host_info);
            }
            std::string response_string;
            resp.SerializeToString(&response_string);

            net_connection *conn = tcp_server::conns[fd];

            if (conn != nullptr) {
                conn->send_message(response_string.c_str(),
                                   response_string.length(),
                                   rins::ID_GetRouteResponse);
            }
        }
    }
}
void subscribe_list::publish(std::vector<uint64_t> &change_mods) {
    pthread_mutex_lock(&_book_list_lock);
    pthread_mutex_lock(&_push_list_lock);

    for (auto it = change_mods.begin(); it != change_mods.end(); it++) {
        uint64_t mod = *it;
        if (_book_list.find(mod) != _book_list.end()) {
            for (auto fds_it = _book_list[mod].begin();
                 fds_it != _book_list[mod].end(); fds_it++) {
                _push_list[*fds_it].insert(mod);
            }
        }
    }

    pthread_mutex_unlock(&_book_list_lock);
    pthread_mutex_unlock(&_push_list_lock);
    server->get_thread_pool()->send_task(push_change_task, this);
}