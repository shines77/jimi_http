
#ifndef JIMI_HTTP_COMMON_H
#define JIMI_HTTP_COMMON_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <stdint.h>

namespace jimi {
namespace http {

struct error_code {
    enum error_code_t {
        Succeed,
        NoErrors,
        InvalidHttpMethod,
        HttpParserError,
    };
    int code;
};

} // namespace http
} // namespace jimi

#endif // JIMI_HTTP_COMMON_H
