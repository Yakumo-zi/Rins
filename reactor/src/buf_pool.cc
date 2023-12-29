#include "buf_pool.h"
#include "io_buf.h"
#include <cassert>
#include <cstdio>
#include <pthread.h>

#define str(cap) #cap

buf_pool *buf_pool::_instance = nullptr;

//用于保证创建单例的init方法只执行一次的锁
pthread_once_t buf_pool::_once = PTHREAD_ONCE_INIT;

// 用于保护内存池链表修改的互斥锁
pthread_mutex_t buf_pool::_mutex = PTHREAD_MUTEX_INITIALIZER;

// buf_pool -->  [m4K] --> io_buf-io_buf-io_buf-io_buf...
//              [m16K] --> io_buf-io_buf-io_buf-io_buf...
//              [m64K] --> io_buf-io_buf-io_buf-io_buf...
//              [m256K] --> io_buf-io_buf-io_buf-io_buf...
//              [m1M] --> io_buf-io_buf-io_buf-io_buf...
//              [m4M] --> io_buf-io_buf-io_buf-io_buf...
//              [m8M] --> io_buf-io_buf-io_buf-io_buf..
void buf_pool::init_memmory(int cap, int num, const char *capstr) {
    io_buf *prev;
    _pool[cap] = new io_buf(cap);
    if (_pool[cap] == nullptr) {
        fprintf(stderr, "buf_pool::constructor::%s new io_buf error\n", capstr);
        exit(1);
    }
    prev = _pool[cap];
    for (int i = 1; i < num; i++) {
        prev->next = new io_buf(cap);
        if (prev->next == nullptr) {
            fprintf(stderr, "buf_pool::constructor::%s new io_buf error\n",
                    capstr);
            exit(1);
        }
        prev = prev->next;
    }
    _total_mem += cap / 1024 * num;
}
buf_pool::buf_pool() : _total_mem(0) {
    init_memmory(m4K, 5000, str(m4k));
    init_memmory(m16K, 1000, str(m16K));
    init_memmory(m64K, 500, str(m64K));
    init_memmory(m256K, 200, str(m256K));
    init_memmory(m1M, 50, str(m1M));
    init_memmory(m4M, 20, str(m4M));
    init_memmory(m8M, 10, str(m8M));
}

io_buf *buf_pool::alloc_buf(int N) {
    // 1 找到N最接近哪hash 组
    int index;
    if (N <= m4K) {
        index = m4K;
    } else if (N <= m16K) {
        index = m16K;
    } else if (N <= m64K) {
        index = m64K;
    } else if (N <= m256K) {
        index = m256K;
    } else if (N <= m1M) {
        index = m1M;
    } else if (N <= m4M) {
        index = m4M;
    } else if (N <= m8M) {
        index = m8M;
    } else {
        return NULL;
    }
    // 2 如果该组已经没有，需要额外申请，那么需要加锁保护
    pthread_mutex_lock(&_mutex);
    if (_pool[index] == NULL) {
        if (_total_mem + index / 1024 >= EXTRA_MEM_LIMIT) {
            //当前的开辟的空间已经超过最大限制
            fprintf(stderr, "buf_pool::alloc already use too many memory!\n");
            exit(1);
        }

        io_buf *new_buf = new io_buf(index);
        if (new_buf == NULL) {
            fprintf(stderr, "buf_pool::alloc new io_buf error\n");
            exit(1);
        }
        _total_mem += index / 1024;
        pthread_mutex_unlock(&_mutex);
        return new_buf;
    }

    // 3 从pool中摘除该内存块
    io_buf *target = _pool[index];
    _pool[index] = target->next;
    pthread_mutex_unlock(&_mutex);

    target->next = NULL;

    return target;
}

void buf_pool::revert(io_buf *buf) {
    int index = buf->capacity;
    buf->length = 0;
    buf->head = 0;
    pthread_mutex_lock(&_mutex);
    assert(_pool.find(index) != _pool.end());
    buf->next = _pool[index];
    _pool[index] = buf->next;
    pthread_mutex_unlock(&_mutex);
}