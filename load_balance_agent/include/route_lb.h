#pragma once

#include "load_balance.h"
#include "rins.pb.h"
#include <bits/stdint-uintn.h>
#include <mutex>
#include <unordered_map>
#include <thread>

class route_lb {
  public:
    route_lb(int id);

    // agent获取一个host主机，将返回的主机结果存放在resp中
    int get_host(int modid, int cmdid, rins::GetHostResponse &resp);

    //根据Dns Service返回的结果更新自己的_route_map
    int update_host(int modid, int cmdid, rins::GetRouteResponse &resp);

  private:
    int _id;
    std::mutex _mutex;
    std::unordered_map<uint64_t, load_balance *> _route_map;
};