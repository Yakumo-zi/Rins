#include "route_lb.h"
#include "load_balance.h"
#include "rins.pb.h"
#include <bits/stdint-uintn.h>
#include <mutex>
#include <pthread.h>
#include <thread>

route_lb::route_lb(int id) : _id(id) {}

int route_lb::get_host(int modid, int cmdid, rins::GetHostResponse &resp) {
    int ret = rins::RET_SUCC;

    uint64_t key = ((uint64_t)modid << 32) + cmdid;
    std::lock_guard<std::mutex> lg(_mutex);

    if (_route_map.find(key) != _route_map.end()) {
        load_balance *lb = _route_map[key];
        if (lb->empty()) {
            assert(lb->status == load_balance::PULLING);
            resp.set_retcode(rins::RET_NOEXIST);
        } else {
            ret = lb->choice_one_host(resp);
            resp.set_retcode(ret);
        }
    } else {
        load_balance *lb = new load_balance(modid, cmdid);
        if (lb == nullptr) {
            fprintf(stderr, "no more space to create loadbalance\n");
            exit(1);
        }
        _route_map[key] = lb;
        lb->pull();
        resp.set_retcode(rins::RET_NOEXIST);
        ret = rins::RET_NOEXIST;
    }
    return ret;
}
int route_lb::update_host(int modid, int cmdid, rins::GetRouteResponse &rsp) {
    // 1. 得到key
    uint64_t key = ((uint64_t)modid << 32) + cmdid;

    std::lock_guard<std::mutex> lg(_mutex);
    // 2. 在_route_map中找到对应的key
    if (_route_map.find(key) != _route_map.end()) {

        load_balance *lb = _route_map[key];

        if (rsp.host_size() == 0) {
            // 2.1 如果返回的结果 lb下已经没有任何host信息，则删除该key
            delete lb;
            _route_map.erase(key);
        } else {
            // 2.2 更新新host信息
            lb->update(rsp);
        }
    }
    return 0;
}