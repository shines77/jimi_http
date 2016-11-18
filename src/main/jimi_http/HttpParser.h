
#ifndef JIMI_HTTP_PARSER_H
#define JIMI_HTTP_PARSER_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <stdint.h>
#include <assert.h>
#include <cstddef>

#include "jimi_http/HttpCommon.h"
#include "jimi_http/StringRef.h"
#include "jimi_http/HttpRequest.h"
#include "jimi_http/HttpResponse.h"

namespace jimi {
namespace http {

struct error_code {
    enum error_code_t {
        kNoErrors,
        kErrorInvalidHttpMethod,
        kErrorHttpParser,
    };
};

class ParseErrorCode {
private:
    int ec_;

public:
    ParseErrorCode() : ec_(0) {}
    ~ParseErrorCode() {}
};

class HttpParser {
public:
    typedef std::uint32_t hash_type;

private:
    int http_code_;
    uint32_t http_version_;
    uint32_t request_method_;

    StringRef http_method_ref_;
    StringRef http_url_ref_;
    StringRef http_version_ref_;

public:
    HttpParser() {
        http_version_ = HttpVersion::HTTP_UNDEFINED;
        request_method_ = HttpRequest::UNDEFINED;
    }

    ~HttpParser() {}

    uint32_t getHttpVersion() const {
        return http_version_;
    }

    void setHttpVersion(uint32_t http_version) {
        http_version_ = http_version;
    }

    uint32_t getRequestMethod() const {
        return request_method_;
    }

    void setRequestMethod(uint32_t request_method) {
        request_method_ = request_method;
    }

    const char * skipWhiteSpaces(const char * data) {
        while (*data == ' ') {
            data++;
        }
        return data;
    }

    const char * nextAndSkipWhiteSpaces(const char * data) {
        data++;
        while (*data == ' ') {
            data++;
        }
        return data;
    }

    const char * nextAndSkipWhiteSpaces_CrLf(const char * data) {
        data++;
        while (*data == ' ' || *data == '\r' || *data == '\n') {
            data++;
        }
        return data;
    }

    const char * getToken(const char * data, char delimiter) {
        const char * cur = data;
        while (*cur != delimiter && *cur != '\0') {
            cur++;
        }
        return cur;
    }

    const char * getToken_CrLf(const char * data) {
        const char * cur = data;
        while (*cur != '\r' && *cur != '\n' && *cur != ' ' && *cur != '\0') {
            cur++;
        }
        return cur;
    }   

    const char * getTokenAndHash(const char * data, char delimiter, hash_type & hash) {
        static const hash_type kSeed_Time31 = 31U;
        hash = 0;
        const char * cur = data;
        while (*cur != delimiter && *cur != '\0') {
            hash += static_cast<hash_type>(*cur) * kSeed_Time31;
            cur++;
        }
        return cur;
    }

    const char * parseHttpMethodAndHash(const char * data, size_t len) {
        hash_type hash;
        const char * cur = getTokenAndHash(data, ' ', hash);
        assert(cur != nullptr);
        assert(cur >= data);
        http_method_ref_.set(data, cur - data);
        return cur;
    }

    const char * parseHttpUrl(const char * data, size_t len) {
        const char * cur = getToken(data, ' ');
        assert(cur != nullptr);
        assert(cur >= data);
        http_url_ref_.set(data, cur - data);
        return cur;
    }

    const char * parseHttpVersion(const char * data, size_t len) {
        const char * cur = getToken_CrLf(data);
        assert(cur != nullptr);
        assert(cur >= data);
        http_version_ref_.set(data, cur - data);
        return cur;
    }

    // Parse http header
    int parseHeader(const char * data, size_t len) {
        int ec = 0;
        const char * cur = data;
        // Http method must be upper case letters.
        if (*cur >= 'A' && *cur <= 'Z') {
            cur = parseHttpMethodAndHash(cur, len);
            if (!cur) {
                ec = error_code::kErrorInvalidHttpMethod;
                goto parse_error;
            }
            cur = nextAndSkipWhiteSpaces(cur);

            cur = parseHttpUrl(cur, len);
            cur = nextAndSkipWhiteSpaces(cur);

            cur = parseHttpVersion(cur, len);
            cur = nextAndSkipWhiteSpaces_CrLf(cur);
        }
        else {
            ec = error_code::kErrorInvalidHttpMethod;
        }
parse_error:
        return ec;
    }

    int parse(const char * data, size_t len) {
        int ec = 0;
        const char * cur = data;
        ec = parseHeader(cur, len);
        return ec;
    }
};

} // namespace http
} // namespace jimi

#endif // !JIMI_HTTP_PARSER_H
