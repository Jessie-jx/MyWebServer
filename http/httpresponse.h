#ifndef HTTP_RESPONSE_H
#define HTTP_RESPONSE_H

#include <unordered_map>
#include <fcntl.h>    // open
#include <unistd.h>   // close
#include <sys/stat.h> // stat
#include <sys/mman.h> // mmap, munmap

#include "../buffer/buffer.h"
using namespace std;

class HttpResponse
{
public:
    HttpResponse();
    ~HttpResponse();
    void Init(const string &srcDir, string &path, bool isKeepAlive = false, int code = -1);
    void makeResponse(Buffer &buff);
    // makeResponse之后的结果：状态行、头字段被写入buff里，文档被被映射到mmFile_
    void UnmapFile();
    char *File();
    size_t FileLen() const;
    void ErrorContent(Buffer &buff, string message);
    int Code() const { return code_; }

private:
    void AddStateLine_(Buffer &buff);
    void AddHeader_(Buffer &buff);
    void AddContent_(Buffer &buff);

    void ErrorHtml_();
    string GetFileType_();

    int code_;
    bool isKeepAlive_;
    string path_;
    string srcDir_;

    char *mmFile_;           // 文件被映射到的内存区域的指针
    struct stat mmFileStat_; // 描述linux文件系统中的文件属性的结构

    static const unordered_map<string, string> SUFFIX_TYPE;
    static const unordered_map<int, string> CODE_STATUS;
    static const unordered_map<int, string> CODE_PATH;
};

#endif