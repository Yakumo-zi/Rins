#include "load_balance.h"
#include "host_info.h"
#include "main_server.h"
#include "rins.pb.h"
#include <bits/stdint-uintn.h>
#include <cassert>
#include <list>
#include <set>

static void get_host_from_list(rins::GetHostResponse &resp,
                               std::list<host_info *> l) {
    host_info *host = l.front();
    rins::HostInfo *hip = resp.mutable_host();
    hip->set_ip(host->ip);
    hip->set_port(host->port);

    l.pop_front();
    l.push_back(host);
}

bool load_balance::empty() const {
    return _idle_list.empty() && _overload_list.empty();
}
int load_balance::choice_one_host(rins::GetHostResponse &rsp) {
    if (_idle_list.empty()) {
        if (_access_cnt >= 10) {
            _access_cnt = 0;
            get_host_from_list(rsp, _overload_list);
        } else {
            ++_access_cnt;
            return rins::RET_OVERLOAD;
        }
    } else {
        if (_overload_list.empty()) {
            get_host_from_list(rsp, _idle_list);
        } else {
            if (_access_cnt >= 10) {
                _access_cnt = 0;
                get_host_from_list(rsp, _overload_list);
            } else {
                ++_access_cnt;
                get_host_from_list(rsp, _idle_list);
            }
        }
    }
    return rins::RET_SUCC;
}

int load_balance::pull() {
    rins::GetRouteRequest req;
    req.set_modid(_modid);
    req.set_cmdid(_cmdid);

    dns_queue->send(req);

    status = PULLING;

    return 0;
}

void load_balance::update(rins::GetRouteResponse &resp) {
    assert(resp.host_size() != 0);
    std::set<uint64_t> remote_hosts;
    std::set<uint64_t> need_delete;

    for (int i = 0; i < resp.host_size(); i++) {
        const rins::HostInfo &h = resp.host(i);

        uint64_t key = ((uint64_t)h.ip() << 32) + h.port();
        remote_hosts.insert(key);

        if (_host_map.find(key) != _host_map.end()) {
            host_info *hi = new host_info(h.ip(), h.port(), 0);
            if (hi == NULL) {
                fprintf(stderr, "new host_info error!\n");
                exit(1);
            }
            _host_map[key] = hi;

            //新增的host信息加入到 空闲列表中
            _idle_list.push_back(hi);
        }
    }
    for (const auto &it : _host_map) {
        if (remote_hosts.find(it.first) == remote_hosts.end()) {
            need_delete.insert(it.first);
        }
    }
    for (auto it : need_delete) {
        uint64_t key = it;

        host_info *hi = _host_map[key];
        if (hi->overload) {
            _overload_list.remove(hi);
        } else {
            _idle_list.remove(hi);
        }
        delete hi;
    }
}