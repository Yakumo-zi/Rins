#ifndef SUBSCRIBE_H
#define SUBSCRIBE_H
#include "rins.pb.h"
#include "rins_reactor.h"
#include <bits/stdint-uintn.h>
#include <pthread.h>
#include <unordered_map>
#include <unordered_set>
#include <vector>

class subscribe_list {
  public:
    static void init() { _instance = new subscribe_list(); }
    static subscribe_list *instance() {
        pthread_once(&_once, init);
        return _instance;
    }

    void subscribe(uint64_t mod, int fd);
    void unsubscribe(uint64_t mod, int fd);
    void publish(std::vector<uint64_t> &change_mods);
    void make_publish_map(
        std::unordered_set<int> &online_fds,
        std::unordered_map<int, std::unordered_set<uint64_t>> &need_publish);

  private:
    subscribe_list();
    subscribe_list(const subscribe_list &);
    const subscribe_list &operator=(const subscribe_list &);

    static subscribe_list *_instance;
    static pthread_once_t _once;
    std::unordered_map<uint64_t, std::unordered_set<int>> _book_list;
    std::unordered_map<int, std::unordered_set<uint64_t>> _push_list;
    pthread_mutex_t _book_list_lock;
    pthread_mutex_t _push_list_lock;
};

#endif