
#ifndef JIMI_HTTP_COMMON_H
#define JIMI_HTTP_COMMON_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <stdint.h>

namespace jimi {
namespace http {

struct HttpVersion {
    enum version {
        HTTP_UNDEFINED = 0,
        HTTP_1_0 = 0x00010000,
        HTTP_1_1 = 0x00010001,
    };

    static uint32_t makeVersion(uint32_t major, uint32_t minor) {
        return ((major << 4) | (minor & 0xFFFFul));
    }

    static uint32_t getMajor(uint32_t http_version) {
        return (http_version >> 4);
    }

    static uint32_t getMinor(uint32_t http_version) {
        return (http_version & 0xFFFFul);
    }
};

} // namespace http
} // namespace jimi

#endif // !JIMI_HTTP_COMMON_H
