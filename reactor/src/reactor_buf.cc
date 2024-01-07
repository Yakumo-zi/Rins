#include "reactor_buf.h"
#include "buf_pool.h"
#include "io_buf.h"
#include <cassert>
#include <cerrno>
#include <cstdio>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

reactor_buf::reactor_buf() { _buf = nullptr; }
reactor_buf::~reactor_buf() { clear(); }

const int reactor_buf::length() const {
    return _buf != nullptr ? _buf->length : 0;
}

void reactor_buf::pop(int len) {
    assert(_buf != nullptr && len <= _buf->length);
    _buf->pop(len);
    if (_buf->length == 0) {
        buf_pool::instance()->revert(_buf);
        _buf = nullptr;
    }
}

void reactor_buf::clear() {
    if (_buf != nullptr) {
        buf_pool::instance()->revert(_buf);
        _buf = nullptr;
    }
}

int input_buf::read_data(int fd) {
    int need_read;
    if (ioctl(fd, FIONREAD, &need_read) == -1) {
        fprintf(stderr, "input_buf::read_data ioctl FIONREAD\n");
        return -1;
    }
    if (_buf == nullptr) {
        _buf = buf_pool::instance()->alloc_buf(need_read);
        if (_buf == nullptr) {
            fprintf(stderr, "input_buf::read_data no idle buf for alloc\n");
            return -1;
        }
    } else {
        assert(_buf->head == 0);
        if (_buf->capacity - _buf->length < (int)need_read) {
            io_buf *new_buf =
                buf_pool::instance()->alloc_buf(need_read + _buf->length);
            if (new_buf == nullptr) {
                fprintf(stderr, "input_buf::read_data no idle buf for alloc\n");
                return -1;
            }
            new_buf->copy(_buf);
            buf_pool::instance()->revert(_buf);
            _buf = new_buf;
        }
    }
    int already_read = 0;
    do {
        if (need_read == 0) {
            already_read = read(fd, _buf->data + _buf->length, m4K);
        } else {
            already_read = read(fd, _buf->data + _buf->length, need_read);
        }
    } while (already_read == -1 && errno == EINTR);

    if (already_read > 0) {
        if (need_read != 0) {
            assert(already_read == need_read);
        }
        _buf->length += already_read;
    }
    return already_read;
}
const char *input_buf::data() const {
    return _buf != nullptr ? _buf->data + _buf->head : nullptr;
}

void input_buf::adjust() {
    if (_buf != nullptr) {
        _buf->adjust();
    }
}

int output_buf::send_data(const char *data, int datalen) {
    if (_buf == nullptr) {
        _buf = buf_pool::instance()->alloc_buf(datalen);
        if (_buf == nullptr) {
            fprintf(stderr, "output_buf::send_data no idle buf for alloc\n");
            return -1;
        }
    } else {
        assert(_buf->head == 0);
        if (_buf->capacity - _buf->length < datalen) {
            io_buf *new_buf = buf_pool::instance()->alloc_buf(datalen);
            if (new_buf == nullptr) {
                fprintf(stderr,
                        "output_buf::send_data no idle buf for alloc\n");
                return -1;
            }
            new_buf->copy(_buf);
            buf_pool::instance()->revert(_buf);
            _buf = new_buf;
        }
    }
    memcpy(_buf->data + _buf->length, data, datalen);
    _buf->length += datalen;
    return 0;
}

int output_buf::write2fd(int fd) {
    assert(_buf != nullptr && _buf->head == 0);
    int already_write = 0;
    do {
        already_write = write(fd, _buf->data, _buf->length);
    } while (already_write == -1 && errno == EINTR);
    if (already_write > 0) {
        _buf->pop(already_write);
        _buf->adjust();
    }
    if (already_write == -1 && errno == EAGAIN) {
        already_write = 0;
    }
    return already_write;
}