#ifndef DNS_ROUTE_H
#define DNS_ROUTE_H
#include <bits/stdint-uintn.h>
#include <mysql/mysql.h>
#include <pthread.h>
#include <unordered_map>
#include <unordered_set>
#include <vector>

class route {
  public:
    static void init() { _instance = new route(); }
    static route *instance() {
        pthread_once(&_once, init);
        return _instance;
    }
    std::unordered_set<uint64_t> get_hosts(int modid, int cmdid);
    int load_version();
    int load_route_data();
    void swap();
    void load_changes(std::vector<uint64_t> &change_list);
    void remove_changes(bool remove_all);

  private:
    route();
    route(const route &) = delete;
    const route &operator=(const route &) = delete;
    void connect_db();
    void build_maps();

    uint64_t _version;

    static route *_instance;
    static pthread_once_t _once;
    MYSQL _db_conn;
    std::string sql;

    std::unordered_map<uint64_t, std::unordered_set<uint64_t>> *_data_map;
    std::unordered_map<uint64_t, std::unordered_set<uint64_t>> *_temp_map;
    pthread_rwlock_t _map_lock;
};
#endif