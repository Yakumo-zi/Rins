#ifndef IO_BUF_H
#define IO_BUF_H
class io_buf {
  public:
    io_buf(int size);

    void clear();
    // 将已处理的数据清空，未处理的数据提前
    void adjust();

    // 将其他buf里的数据拷贝到当前buf
    void copy(const io_buf *other);

    //处理长度为len的数据，移动head和修正length
    void pop(int len);

    io_buf *next;
    // buffer的容量
    int capacity;
    // 未处理数据的长度
    int length;
    // 未处理数据的头指针
    int head;
    // buffer
    char *data;
};
#endif