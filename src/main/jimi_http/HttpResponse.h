
#ifndef JIMI_HTTP_RESPONSE_H
#define JIMI_HTTP_RESPONSE_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

namespace jimi {
namespace http {

class HttpResponse {
public:
    //
private:
    int error_code_;

public:
    HttpResponse() {}
    ~HttpResponse() {}
};

} // namespace http
} // namespace jimi

#endif // !JIMI_HTTP_RESPONSE_H
