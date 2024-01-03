#ifndef BUF_POOL_H
#define BUF_POOL_H
#include "io_buf.h"
#include <bits/stdint-uintn.h>
#include <pthread.h>
#include <unordered_map>

// 总内存最大为5G
#define EXTRA_MEM_LIMIT (5U * 1024 * 1024)

enum MEM_CAP {
    m4K = 4096,
    m16K = 16384,
    m64K = 65536,
    m256K = 262144,
    m1M = 1048576,
    m4M = 4194304,
    m8M = 8388608,
};

//TODO 性能优化
class buf_pool {
  public:
    static void init() { _instance = new buf_pool(); }
    static buf_pool *instance() {
        pthread_once(&_once, init);
        return _instance;
    }
    io_buf *alloc_buf(int N);
    io_buf *alloc_buf() { return alloc_buf(m4K); }
    void revert(io_buf *buf);

  private:
    buf_pool();
    void init_memmory(int cap,int num,const char* capstr);
    //拷贝构造私有化
    buf_pool(const buf_pool &);
    const buf_pool &operator=(const buf_pool &);

  private:
    static buf_pool *_instance;
    static pthread_once_t _once;
    static pthread_mutex_t _mutex;
    std::unordered_map<int, io_buf *> _pool;
    uint64_t _total_mem;
};
#endif