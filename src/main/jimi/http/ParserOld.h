
#ifndef JIMI_HTTP_PARSER_H
#define JIMI_HTTP_PARSER_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <stdint.h>
#include <assert.h>
#include <cstddef>
#include <iostream>

#include "jimi/http/Common.h"
#include "jimi/http/StringRef.h"
#include "jimi/http/StringRefList.h"
#include "jimi/http/Request.h"
#include "jimi/http/Response.h"

using namespace std;

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

template <std::size_t InitContentSize = 1024>
class HttpParser {
public:
    typedef std::uint32_t hash_type;

    // kInitContentSize minimize value is 256.
    static const std::size_t kMinContentSize = 256;
    // kInitContentSize = max(InitContentSize, 256);
    static const std::size_t kInitContentSize = (InitContentSize > kMinContentSize) ? InitContentSize : kMinContentSize;

private:
    int status_code_;
    uint32_t http_version_;
    uint32_t request_method_;

    StringRef http_method_ref_;
    StringRef http_url_ref_;
    StringRef http_version_ref_;
    std::size_t content_length_;
    std::size_t content_size_;
    const char * content_;
    StringRefList<16> entries_;
    char inner_content_[kInitContentSize];

public:
    HttpParser() : status_code_(0),
        http_version_(HttpVersion::HTTP_UNDEFINED),
        request_method_(HttpRequest::UNDEFINED),
        content_length_(0),
        content_size_(0), content_(nullptr) {
        //
    }

    ~HttpParser() {
        if (content_) {
            delete[] content_;
            content_ = nullptr;
        }
    }

    std::size_t getEntrySize() const {
        return entries_.size();
    }

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

    template <char delimiter = ' '>
    const char * skipWhiteSpaces(const char * data) {
        assert(data != nullptr);
        while (*data == delimiter || *data == ' ') {
            data++;
        }
        return data;
    }

    template <char delimiter = ' '>
    const char * nextAndSkipWhiteSpaces(const char * data) {
        assert(data != nullptr);
        data++;
        return skipWhiteSpaces<delimiter>(data);
    }

    const char * nextAndSkipWhiteSpaces_CrLf(const char * data) {
        assert(data != nullptr);
        data++;
        while (*data == ' ' || *data == '\r' || *data == '\n') {
            data++;
        }
        return data;
    }

    template <char delimiter = ' '>
    const char * getToken(const char * data) {
        const char * cur = data;
        assert(cur != nullptr);
        while (*cur != delimiter && *cur != ' ' && *cur != '\0') {
            cur++;
        }
        return cur;
    }

    template <char delimiter = ' '>
    const char * getTokenAndHash(const char * data, hash_type & hash) {
        static const hash_type kSeed_Time31 = 31U;
        hash = 0;
        const char * cur = data;
        assert(cur != nullptr);
        while (*cur != delimiter && *cur != ' ' && *cur != '\0') {
            hash += static_cast<hash_type>(*cur) * kSeed_Time31;
            cur++;
        }
        return cur;
    }

    template <char delimiter = ' '>
    const char * getToken_CrLf(const char * data) {
        const char * cur = data;
        assert(cur != nullptr);
        while (*cur != '\r' && *cur != '\n' && *cur != delimiter && *cur != ' ' && *cur != '\0') {
            cur++;
        }
        return cur;
    }

    const char * getKeynameToken(const char * data) {
        const char * cur = data;
        assert(cur != nullptr);
        while (*cur != ':' && *cur != ' ' && *cur != '\0') {
            cur++;
        }
        return cur;
    }

    const char * getValueToken(const char * data) {
        const char * cur = data;
        assert(cur != nullptr);
        while (*cur != '\r' && *cur != '\n' && *cur != '\0') {
            cur++;
        }
        return cur;
    }

    const char *  checkAndSkipCrLf(const char * data, bool & is_end) {
        const char * cur = data;
        assert(cur != nullptr);
        is_end = false;
        do {
            if (*cur == '\r') {
                if (*(cur + 2) != '\r') {
                    if (*(cur + 1) == '\n') {
                        return (cur + 2);   // "\r\n", mostly is this case.
                    }
                    else {
                        return (cur + 1);   // "\r" only
                    }
                }
                else {
                    if (*(cur + 1) == '\n') {
                        if (*(cur + 3) == '\n') {
                            is_end = true;      // "\r\n\r\n"
                            return (cur + 4);
                        }
                        else {
                            return (cur + 3);   // "\r\n\r"+"XXXXXX", May be is a error format.
                        }
                    }
                    else {
                        return (cur + 1);       // "\r" only
                    }
                }
            }
            else if (*cur == '\n') {
                return (cur + 1);   // "\n" only
            }
            else if (*cur == ' ') {
                cur++;              // skip whitespaces
                continue;
            }
            else if (*cur == '\0') {
                is_end = true;
                return cur;
            }
            else {
                break;
            }
        } while (1);
        return cur;
    }

    const char * parseHttpMethod(const char * data) {
        const char * cur = getToken<' '>(data);
        assert(cur != nullptr);
        assert(cur >= data);
        http_method_ref_.set(data, cur - data);
        return cur;
    }

    const char * parseHttpMethodAndHash(const char * data) {
        hash_type hash;
        const char * cur = getTokenAndHash<' '>(data, hash);
        assert(cur != nullptr);
        assert(cur >= data);
        http_method_ref_.set(data, cur - data);
        return cur;
    }

    const char * parseHttpURI(const char * data) {
        const char * cur = getToken<' '>(data);
        assert(cur != nullptr);
        assert(cur >= data);
        http_url_ref_.set(data, cur - data);
        return cur;
    }

    const char * parseHttpVersion(const char * data) {
        const char * cur = getToken_CrLf<' '>(data);
        assert(cur != nullptr);
        assert(cur >= data);
        http_version_ref_.set(data, cur - data);
        return cur;
    }

    const char * parseHttpEntryList(const char * data) {
        const char * last;
        const char * cur = data;
        do {
            // Skip the whitespaces ahead of every entry.
            //cur = nextAndSkipWhiteSpaces<' '>(cur);

            last = cur;
            cur = getKeynameToken(cur);
            const char * key_name = last;
            std::size_t key_len = cur - last;

            cur = nextAndSkipWhiteSpaces<':'>(cur);
            last = cur;

            cur = getValueToken(cur);
            const char * value_name = last;
            std::size_t value_len = cur - last;

            // Append the key and value pair to StringRefList.
            //entries_.append(key_name, key_len, value_name, value_len);

            cur = skipWhiteSpaces<' '>(cur);
            bool is_end;
            cur = checkAndSkipCrLf(cur, is_end);
            if (is_end)
                break;
        } while (1);
        return cur;
    }

    // Parse http header
    int parseHeader(const char * data, size_t len) {
        int ec = 0;
        const char * cur = data;
        // Skip the whitespaces ahead of http header.
        cur = skipWhiteSpaces(cur);
        // Http method characters must be upper case letters.
        if (*cur >= 'A' && *cur <= 'Z') {
            //cur = parseHttpMethodAndHash(cur);
            cur = parseHttpMethod(cur);
            if (!cur) {
                ec = error_code::kErrorInvalidHttpMethod;
                goto parse_error;
            }
            cur = nextAndSkipWhiteSpaces(cur);

            cur = parseHttpURI(cur);
            cur = nextAndSkipWhiteSpaces(cur);

            cur = parseHttpVersion(cur);
            cur = nextAndSkipWhiteSpaces_CrLf(cur);

            assert(cur >= data);
            assert(len >= (std::size_t)(cur - data));
            entries_.setRef(cur, len - (cur - data));

            cur = parseHttpEntryList(cur);
        }
        else {
            ec = error_code::kErrorInvalidHttpMethod;
        }
parse_error:
        return ec;
    }

    int parse(const char * data, size_t len) {
        int ec = 0;
        const char * content;
        // Copy the input http header data.
        content_size_ = len;
        if (len < kInitContentSize) {
            ::memcpy((void *)&inner_content_[0], data, len);
            inner_content_[len] = '\0';
            if (content_)
                content_ = nullptr;
            content = const_cast<const char * >(&inner_content_[0]);
        }
        else {
            char * new_content = new char [len + 1];
            if (new_content != nullptr) {
                ::memcpy((void *)new_content, data, len);
                new_content[len] = '\0';
                content_ = const_cast<const char *>(new_content);
                content = content_;
            }
            else {
                return error_code::kErrorHttpParser;
            }
        }
        // Start parse the http header.
        ec = parseHeader(content, len);
        return ec;
    }

    void displayEntries() {
        std::cout << "Http entries: (length = " << entries_.ref.size() << " bytes)" << std::endl << std::endl;
        std::cout << entries_.ref.c_str() << std::endl;

        std::cout << "Http entries size: " << entries_.size() << std::endl << std::endl;
        std::size_t data_len = entries_.ref.size();
        for (std::size_t i = 0; i < entries_.size(); ++i) {
            if ((entries_.items[i].key.offset <= data_len)
                && ((entries_.items[i].key.offset + entries_.items[i].key.length) < data_len)
                && (entries_.items[i].value.offset <= data_len)
                && ((entries_.items[i].value.offset + entries_.items[i].value.length) < data_len)) {
                std::string key(entries_.ref.data() + entries_.items[i].key.offset, entries_.items[i].key.length);
                std::string value(entries_.ref.data() + entries_.items[i].value.offset, entries_.items[i].value.length);
                std::cout << "key = " << key.c_str() << ", value = " << value.c_str() << std::endl;
            }
            else {
                std::cout << "Error entry." << std::endl;
            }
        }
        std::cout << std::endl;
    }
};

} // namespace http
} // namespace jimi

#endif // !JIMI_HTTP_PARSER_H
