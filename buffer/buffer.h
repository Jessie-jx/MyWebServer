#ifndef BUFFER_H
#define BUFFER_H
#include <vector>
#include <atomic>
#include "sys/uio.h" // iovec readv
#include "assert.h"
using namespace std;

class Buffer
{
public:
    Buffer(int initBuffSize = 1024);
    ~Buffer() = default; // =default 表示使用编译器默认实现的成员函数

    size_t WritableBytes() const;
    size_t ReadableBytes() const;
    size_t PrependableBytes() const;

    const char *Peek() const;
    void MakeSpace_(size_t len);
    void EnsureWriteable(size_t len);
    void HasWritten(size_t len);

    char *BeginWrite();
    const char *BeginWrite() const;

    void Retrieve(size_t len);
    void RetrieveUntil(const char *end);
    void RetrieveAll();
    string RetrieveAllToStr();

    void Append(const std::string &str);
    void Append(const char *str, size_t len);
    void Append(const void *data, size_t len);
    void Append(const Buffer &buff);

    ssize_t ReadFd(int fd, int *Errno);
    ssize_t WriteFd(int fd, int *Errno);

private:
    char *BeginPtr_();
    const char *BeginPtr_() const;
    vector<char> buffer_;
    atomic<size_t> readPos_;
    atomic<size_t> writePos_;
};

#endif