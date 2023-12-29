#ifndef REACTOR_BUF_H
#define REACTOR_BUF_H

#include "io_buf.h"
class reactor_buf {
  public:
    reactor_buf();
    ~reactor_buf();
    const int length() const;
    void pop(int len);
    void clear();

  protected:
    io_buf *_buf;
};
class input_buf : public reactor_buf {
  public:
    int read_data(int fd);
    const char *data() const;
    void adjust();
};
class output_buf : public reactor_buf {
  public:
    int send_data(const char *data, int datalen);
    int write2fd(int fd);
};

#endif