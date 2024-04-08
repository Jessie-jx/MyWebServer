#ifndef HTTP_REQUEST_H
#define HTTP_REQUEST_H

#include "../buffer/buffer.h"
#include <unordered_map>
#include <unordered_set>
#include <regex>
#include <string>
#include <errno.h>

using namespace std;

class HttpRequest
{
public:
    enum PARSE_STATE
    {
        REQUEST_LINE,
        HEADERS,
        BODY,
        FINISH,
    };
    HttpRequest() { Init(); }
    ~HttpRequest() = default;
    void Init();
    bool parse(Buffer &buff);

    string path() const;
    string &path();

    bool IsKeepAlive() const;

private:
    bool ParseRequestLine_(const string &line);
    void ParseHeader_(const string &line);
    // bool ParseBody_(const string &line);

    void ParsePath_();

    PARSE_STATE state_;
    string method_, path_, version_, body_;
    unordered_map<string, string> header_;
    unordered_map<string, string> post_;

    static const unordered_set<string> DEFAULT_HTML;
    static const unordered_map<string, int> DEFAULT_HTML_TAG;
};

#endif