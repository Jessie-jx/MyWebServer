#include "buffer.h"

Buffer::Buffer(int initBuffSize) : buffer_(initBuffSize), readPos_(0), writePos_(0) {}

size_t Buffer::WritableBytes() const
{
    return buffer_.size() - writePos_;
}
size_t Buffer::ReadableBytes() const
{
    return writePos_ - readPos_;
}
size_t Buffer::PrependableBytes() const
{
    return readPos_;
}

// 可读区的起点
const char *Buffer::Peek() const
{
    return BeginPtr_() + readPos_;
}

const char *Buffer::BeginWriteConst() const
{
    return BeginPtr_() + writePos_;
}

char *Buffer::BeginWrite()
{
    return BeginPtr_() + writePos_;
}

void Buffer::HasWritten(size_t len)
{
    writePos_ += len;
}

void Buffer::Retrieve(size_t len)
{
    assert(len <= ReadableBytes());
    readPos_ += len;
}
void Buffer::RetrieveUntil(const char *end)
{
    assert(Peek() <= end);
    Retrieve(end - Peek());
}
void Buffer::RetrieveAll()
{
    // 每次全部读完的时候，readPos_和writePos_就回到原点
    bzero(&buffer_[0], buffer_.size());
    readPos_ = 0;
    writePos_ = 0;
}
string Buffer::RetrieveAllToStr()
{
    string str(Peek(), ReadableBytes());
    RetrieveAll();
    return str;
}

void Buffer::Append(const std::string &str)
{
    Append(str.data(), str.length());
}

void Buffer::Append(const Buffer &buff)
{
    Append(buff.Peek(), buff.ReadableBytes());
}

void Buffer::Append(const void *data, size_t len)
{
    assert(data);
    Append(static_cast<const char *>(data), len);
}

void Buffer::Append(const char *str, size_t len)
{
    assert(str); // assert检查是否为空指针
    EnsureWriteable(len);
    copy(str, str + len, BeginWrite());
    HasWritten(len);
}

void Buffer::EnsureWriteable(size_t len)
{
    // 确保从writePos_开始的buffer_里有len这么大的空间
    if (WritableBytes() < len)
    {
        // 说明需要扩容
        MakeSpace_(len);
    }
    assert(WritableBytes() >= len);
}

ssize_t Buffer::ReadFd(int fd, int *Errno)
{
    char buff[65535];
    struct iovec[2];
    size_t writable = WritableBytes();
    iovec[0].iov_base = BeginPtr_() + writePos_;
    iovec[0].iov_len = writable;
    iovec[1].iov_base = buff;
    iovec[1].iov_len = sizeof(buff);

    ssize_t len = readv(fd, iovec, 2);
    if (len < 0)
    {
        *Errno = errno;
    }
    else if (static_cast<size_t>(len) <= writable)
    {
        writePos_ += len;
    }
    else
    {
        writePos_ = buffer_.size();
        Append(buff, len - writable);
    }
    return len;
}

ssize_t Buffer::WriteFd(int fd, int *Errno)
{
    size_t readSize = ReadableBytes();
    ssize_t len = write(fd, Peek(), readSize);
    if (len < 0)
    {
        *Errno = errno;
        return len;
    }
    readPos_ += len;
    return len;
}

char *Buffer::BeginPtr_()
{
    return buffer_.begin();
}

const char *Buffer::BeginPtr_() const
{
    return buffer_.begin();
}

void Buffer::MakeSpace_(size_t len)
{
    // 如果可写区和腾挪区加起来够用的话，就不需要扩容vector
    // 把可读区挪动到最前面
    if (PrependableBytes() + WritableBytes() >= len)
    {
        size_t readable = ReadableBytes();
        copy(BeginPtr_() + readPos_, BeginPtr_() + writePos_, BeginPtr_());
        readPos_ = 0;
        writePos_ = readPos_ + readable;
        assert(readable == ReadableBytes());
    }
    else
    {
        // 需要扩容vector
        buffer_.resize(writePos_ + len + 1);
    }
}