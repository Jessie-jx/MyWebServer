#include "httpconn.h"
using namespace std;

const char *HttpConn::srcDir;
std::atomic<int> HttpConn::userCount;
bool HttpConn::isET;

HttpConn::HttpConn()
{
    fd_ = -1;
    addr_ = {0};
    isClose_ = true;
}

HttpConn::~HttpConn()
{
    Close();
}

void HttpConn::init(int sockFd, const sockaddr_in &addr)
{
    assert(sockFd > 0);
    fd_ = sockFd;
    addr_ = addr;
    ++userCount;
    writeBuff_.RetrieveAll();
    readBuff_.RetrieveAll();
    isClose_ = false;
}

void HttpConn::Close()
{
    response_.UnmapFile();
    isClose_ = true;
    --userCount;
    close(fd_);
}

int HttpConn::GetFd() const
{
    return fd_;
};

ssize_t HttpConn::read(int *saveErrno)
{
    ssize_t len = -1;
    do
    {
        len = readBuff_.ReadFd(fd_, saveErrno);
        if (len <= 0)
        {
            break;
        }
    } while (isET); // ET 就要一直读 读完为止
    return len;
}

ssize_t HttpConn::write(int *saveErrno)
{
    ssize_t len = -1;

    do
    {
        // 需要多次才能写出去
        len = writev(fd_, iov_, iovCnt_);
        if (len <= 0)
        {
            *saveErrno = errno;
            break;
        }
        if (iov_[0].iov_len + iov_[1].iov_len == 0)
        {
            break;
        } /* 传输结束 */
        else if (static_cast<size_t>(len) > iov_[0].iov_len)
        {
            // 写出的既包含iov_[0]，也包含iov_[1]
            iov_[1].iov_base = (uint8_t *)iov_[1].iov_base + len - iov_[0].iov_len;
            iov_[1].iov_len -= (len - iov_[0].iov_len);
            if (iov_[0].iov_len)
            {
                writeBuff_.RetrieveAll();
                iov_[0].iov_len = 0;
            }
        }
        else
        {
            // 说明只写出了iov_[0]的部分
            iov_[0].iov_base = (uint8_t *)iov_[0].iov_base + len;
            iov_[0].iov_len -= len;
            writeBuff_.Retrieve(len);
        }

    } while (isET || ToWriteBytes() > 10240);

    return len;
}

bool HttpConn::process()
{
    request_.Init();
    if (readBuff_.ReadableBytes() <= 0)
    {
        return false;
    }
    else if (request_.parse(readBuff_))
    {
        response_.Init(srcDir, request_.path(), request_.IsKeepAlive(), 200);
    }
    else
    {
        response_.Init(srcDir, request_.path(), false, 200);
    }

    response_.makeResponse(writeBuff_);
    // 响应头
    iov_[0].iov_base = const_cast<char *>(writeBuff_.Peek()); // const_cast 去除变量的const属性
    iov_[0].iov_len = writeBuff_.ReadableBytes();
    iovCnt_ = 1;

    // 文件
    if (response_.FileLen() > 0 && response_.File())
    {
        iov_[1].iov_base = response_.File();
        iov_[1].iov_len = response_.FileLen();
        iovCnt_ = 2;
    }
    return true;
}