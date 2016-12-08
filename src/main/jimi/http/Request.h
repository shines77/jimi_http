
#ifndef JIMI_HTTP_REQUEST_H
#define JIMI_HTTP_REQUEST_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

namespace jimi {
namespace http {

class Request {
public:
    /************************************************************************
     *
     * HTTP Request method:
     *
     *   GET, POST, HEAD, PUT, DELETE, OPTIONS, TRACE, CONNECT.
     *
     ************************************************************************/
    enum RequestMethod {
        UNDEFINED,
        GET,
        POST,
        HEAD,
        PUT,
        DELETE,
        OPTIONS,
        TRACE,
        CONNECT
    };

private:
    int error_code_;

public:
    Request() {}
    ~Request() {}
};

} // namespace http
} // namespace jimi

#endif // !JIMI_HTTP_REQUEST_H
