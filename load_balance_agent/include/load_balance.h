#pragma once
#include "host_info.h"
#include "rins.pb.h"
#include <bits/stdint-uintn.h>
#include <list>
#include <unordered_map>
class load_balance {
  public:
    load_balance(int modid, int cmdid) : _modid(modid), _cmdid(cmdid) {
        // load_balance 初始化构造
    }
    //判断是否已经没有host在当前LB节点中
    bool empty() const;
    //从当前的双队列中获取host信息
    int choice_one_host(rins::GetHostResponse &rsp);
    //如果list中没有host信息，需要从远程的DNS Service发送GetRouteHost请求申请
    int pull();
    void update(rins::GetRouteResponse& resp);
    enum STAUTS {
        PULLING,
        NEW,
    };
    STAUTS status;

  private:
    int _modid;
    int _cmdid;
    int _access_cnt;
    std::unordered_map<uint64_t, host_info *> _host_map;
    std::list<host_info *> _idle_list;
    std::list<host_info *> _overload_list;
};