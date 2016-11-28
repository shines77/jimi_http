
#ifndef JIMI_HTTP_PARSER_H
#define JIMI_HTTP_PARSER_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <stdint.h>
#include <assert.h>
#include <cstddef>
#include <iostream>

#include "jimi_http/HttpCommon.h"
#include "jimi_http/InputStream.h"
#include "jimi_http/StringRef.h"
#include "jimi_http/StringRefList.h"
#include "jimi_http/HttpRequest.h"
#include "jimi_http/HttpResponse.h"

using namespace std;

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

    void reset() {
        http_method_ref_.reset();
        http_url_ref_.reset();
        http_version_ref_.reset();
        content_size_ = 0;
        content_ = nullptr;
        entries_.reset();
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
    void skipWhiteSpaces(InputStream & is) {
        assert(is.current() != nullptr);
        while (is.hasNext() && (is.get() == delimiter || is.get() == ' ')) {
            is.next();
        }
    }

#if 0
    template <char delimiter = ' '>
    bool next(InputStream & is) {
        assert(is.current() != nullptr);
        if (is.hasNext()) {
            is.next();
            assert(is.current() != nullptr);
            if (is.hasNext() && is.get() != '\0')
                return true;
        }
        return false;
    }

    template <char delimiter = ' '>
    bool nextAndSkipWhiteSpaces(InputStream & is) {
        assert(is.current() != nullptr);
        if (is.hasNext()) {
            is.next();
            assert(is.current() != nullptr);
            if (is.hasNext() && is.get() != '\0') {
                skipWhiteSpaces<delimiter>(is);
                return true;
            }
        }
        return false;
    }

    bool nextAndSkipWhiteSpaces_CrLf(InputStream & is) {
        assert(is.current() != nullptr);
        bool is_ok = next(is);
        if (is_ok) {
            while (is.hasNext() && (is.get() == ' ' || is.get() == '\r' || is.get() == '\n')) {
                is.next();
            }
        }
        return is_ok;
    }

    template <char delimiter = ' '>
    bool getToken(InputStream & is) {
        assert(is.current() != nullptr);
        while (is.hasNext() && (is.get() != delimiter && is.get() != ' ' && is.get() != '\0')) {
            is.next();
        }
        return true;
    }

    template <char delimiter = ' '>
    bool getTokenAndHash(InputStream & is, hash_type & hash) {
        static const hash_type kSeed_Time31 = 31U;
        hash = 0;
        assert(is.current() != nullptr);
        while (is.hasNext() && (is.get() != delimiter && is.get() != ' ' && is.get() != '\0')) {
            hash += static_cast<hash_type>(is.get()) * kSeed_Time31;
            is.next();
        }
        return true;
    }

    template <char delimiter = ' '>
    bool getToken_CrLf(InputStream & is) {
        assert(is.current() != nullptr);
        while (is.hasNext() && (is.get() != '\r' && is.get() != '\n'
            && is.get() != delimiter && is.get() != ' '
            && is.get() != '\0')) {
            is.next();
        }
        return true;
    }

    bool getKeynameToken(InputStream & is) {
        assert(is.current() != nullptr);
        while (is.hasNext() && (is.get() != ':' && is.get() != ' ' && is.get() != '\0')) {
            is.next();
        }
        return true;
    }

    bool getValueToken(InputStream & is) {
        assert(is.current() != nullptr);
        while (is.hasNext() && (is.get() != '\r' && is.get() != '\n' && is.get() != '\0')) {
            is.next();
        }
        return true;
    }

    bool checkAndSkipCrLf(InputStream & is, bool & is_end) {
        assert(is.current() != nullptr);
        is_end = false;
        if (is.remain() >= 4) {
            // If the remain length is more than or equal 4 bytes, needn't to check the tail.
            do {
                if (is.get() == '\r') {
                    if (is.peek(2) != '\r') {
                        if (is.peek(1) == '\n') {
                            is.moveTo(2);       // "\r\nX", In most cases, it will be walk to this path.
                            return true;
                        }
                        else {
                            is.moveTo(1);       // "\rXX", Maybe it's a wrong format.
                            return false;
                        }
                    }
                    else {
                        if (is.peek(1) == '\n') {
                            if (is.peek(3) == '\n') {
                                is.moveTo(4);   // "\r\n\r\n", It's the end of the http header.
                                is_end = true;
                                return true;
                            }
                            else {
                                is.moveTo(3);   // "\r\n\rX", Maybe it's a wrong format.
                                return false;
                            }
                        }
                        else {
                            is.moveTo(1);       // "\rX\rX", Maybe it's a wrong format.
                            return false;
                        }
                    }
                }
                else if (is.get() == '\n') {
                    is.moveTo(1);       // "\n" only, Maybe it's a wrong format.
                    return false;
                }
                else if (is.get() == ' ') {
                    is.next();          // Skip the whitespaces.
                    continue;
                }
                else if (is.get() == '\0') {
                    is_end = true;
                    return true;
                }
                else {
                    break;
                }
            } while (1);
            return false;
        }
        else {
            do {
                // If the remain length is less than 4 bytes, we need to check the tail.
                if (!is.hasNext())
                    return false;
                if (is.get() == '\r') {
                    if (is.hasNext(2) && is.peek(2) != '\r') {
                        if (is.peek(1) == '\n') {
                            is.moveTo(2);       // "\r\nX", In most cases, it will be walk to this path.
                            return true;
                        }
                        else {
                            is.moveTo(1);       // "\rXX", Maybe it's a wrong format.
                            return false;
                        }
                    }
                    else {
                        if (is.hasNext(1) && is.peek(1) == '\n') {
                            is.moveTo(3);       // "\r\n\r\0", Maybe it's a wrong format.
                            return false;
                        }
                        else {
                            is.moveTo(1);       // "\rX\r\0", Maybe it's a wrong format.
                            return false;
                        }
                    }
                }
                else if (is.get() == '\n') {
                    is.moveTo(1);       // "\n" only, Maybe it's a wrong format.
                    return false;
                }
                else if (is.get() == ' ') {
                    is.next();          // Skip the whitespaces.
                    continue;
                }
                else if (is.get() == '\0') {
                    is_end = true;
                    return true;
                }
                else {
                    break;
                }
            } while (1);
            return false;
        }
    }
#else
    template <char delimiter = ' '>
    bool next(InputStream & is) {
        is.next();
        assert(is.current() != nullptr);
        if (is.hasNext() && is.get() != '\0')
            return true;
        else
            return false;
    }

    template <char delimiter = ' '>
    bool nextAndSkipWhiteSpaces(InputStream & is) {
        is.next();
        assert(is.current() != nullptr);
        if (is.hasNext() && is.get() != '\0') {
            skipWhiteSpaces<delimiter>(is);
            return true;
        }
        return false;
    }

    bool nextAndSkipWhiteSpaces_CrLf(InputStream & is) {
        assert(is.current() != nullptr);
        bool is_ok = next(is);
        if (is_ok) {
            while (is.get() == ' ' || is.get() == '\r' || is.get() == '\n') {
                is.next();
                if (!is.hasNext())
                    return false;
            }
        }
        return is_ok;
    }

    template <char delimiter = ' '>
    bool getToken(InputStream & is) {
        assert(is.current() != nullptr);
        while (is.hasNext() && (is.get() != delimiter && is.get() != ' ' && is.get() != '\0')) {
            is.next();
        }
        return true;
    }

    template <char delimiter = ' '>
    bool getTokenAndHash(InputStream & is, hash_type & hash) {
        static const hash_type kSeed_Time31 = 31U;
        hash = 0;
        assert(is.current() != nullptr);
        while (is.hasNext() && (is.get() != delimiter && is.get() != ' ' && is.get() != '\0')) {
            hash += static_cast<hash_type>(is.get()) * kSeed_Time31;
            is.next();
        }
        return true;
    }

    template <char delimiter = ' '>
    bool getToken_CrLf(InputStream & is) {
        assert(is.current() != nullptr);
        while (is.hasNext() && (is.get() != '\r' && is.get() != '\n'
            && is.get() != delimiter && is.get() != ' '
            && is.get() != '\0')) {
            is.next();
        }
        return true;
    }

    bool getKeynameToken(InputStream & is) {
        assert(is.current() != nullptr);
        while (is.hasNext() && (is.get() != ':' && is.get() != ' ' && is.get() != '\0')) {
            is.next();
        }
        return true;
    }

    bool getValueToken(InputStream & is) {
        assert(is.current() != nullptr);
        while (is.hasNext() && (is.get() != '\r' && is.get() != '\n' && is.get() != '\0')) {
            is.next();
        }
        return true;
    }

    bool checkAndSkipCrLf(InputStream & is, bool & is_end) {
        assert(is.current() != nullptr);
        is_end = false;
scan_start:
        if (is.remain() >= 4) {
            // If the remain length is more than or equal 4 bytes, needn't to check the tail.
            do {
                if (is.get() == '\r') {
                    if (is.peek(2) != '\r') {
                        if (is.peek(1) == '\n') {
                            is.moveTo(2);       // "\r\nX", In most cases, it will be walk to this path.
                            return true;
                        }
                        return false;
                    }
                    else {
                        if (is.peek(1) == '\n') {
                            if (is.peek(3) == '\n') {
                                is.moveTo(4);   // "\r\n\r\n", It's the end of the http header.
                                is_end = true;
                                return true;
                            }
                        }
                        return false;
                    }
                }
                else if (is.get() == ' ') {
                    is.next();          // Skip the whitespaces.
                    goto scan_start;
                }
                else if (is.get() == '\0') {
                    is_end = true;
                    return true;
                }
                else {
                    break;
                }
            } while (1);
        }
        else {
            do {
                // If the remain length is less than 4 bytes, we need to check the tail.
                if (!is.hasNext())
                    return false;
                if (is.get() == '\r') {
                    if (is.hasNext(2) && is.peek(2) != '\r') {
                        if (is.peek(1) == '\n') {
                            is.moveTo(2);       // "\r\nX", In most cases, it will be walk to this path.
                            return true;
                        }
                    }
                    return false;
                }
                else if (is.get() == ' ') {
                    is.next();          // Skip the whitespaces.
                    continue;
                }
                else if (is.get() == '\0') {
                    is_end = true;
                    return true;
                }
                else {
                    break;
                }
            } while (1);
        }
        return false;
    }
#endif

    bool parseHttpMethod(InputStream & is) {
        if (!http_method_ref_.is_empty())
            return false;
        const char * mark = is.current();
        bool is_ok = getToken<' '>(is);
        if (is_ok) {
            assert(is.current() != nullptr);
            assert(is.current() >= mark);
            std::ptrdiff_t len = is.current() - mark;
            http_method_ref_.set(mark, len);
            if (len <= 0)
                return false;
        }
        return is_ok;
    }

    bool parseHttpMethodAndHash(InputStream & is) {
        if (!http_method_ref_.is_empty())
            return false;
        hash_type hash;
        const char * mark = is.current();
        bool is_ok = getTokenAndHash<' '>(is, hash);
        if (is_ok) {
            assert(is.current() != nullptr);
            assert(is.current() >= mark);
            std::ptrdiff_t len = is.current() - mark;
            http_method_ref_.set(mark, len);
            if (len <= 0)
                return false;
        }
        return is_ok;
    }

    bool parseHttpURI(InputStream & is) {
        if (!http_url_ref_.is_empty())
            return false;
        const char * mark = is.current();
        bool is_ok = getToken<' '>(is);
        if (is_ok) {
            assert(is.current() != nullptr);
            assert(is.current() >= mark);
            std::ptrdiff_t len = is.current() - mark;
            http_url_ref_.set(mark, len);
            if (len <= 0)
                return false;
        }
        return is_ok;
    }

    bool parseHttpVersion(InputStream & is) {
        if (!http_version_ref_.is_empty())
            return false;
        const char * mark = is.current();
        bool is_ok = getToken_CrLf<' '>(is);
        if (is_ok) {
            assert(is.current() != nullptr);
            assert(is.current() >= mark);
            static const std::ptrdiff_t lenHTTPVersion = sizeof("HTTP/1.1") - 1;
            std::ptrdiff_t len = is.current() - mark;
            if (len < lenHTTPVersion)
                return false;
            http_version_ref_.set(mark, len);
            if (len <= 0)
                return false;
        }
        return is_ok;
    }

    bool parseHttpEntryList(InputStream & is) {
        const char * mark;
        bool is_ok;
        do {
            // Skip the whitespaces ahead of every entry.
            is_ok = nextAndSkipWhiteSpaces<' '>(is);
            if (!is_ok)
                return false;

            mark = is.current();
            is_ok = getKeynameToken(is);
            if (!is_ok)
                return false;
            const char * key_name = mark;
            std::size_t key_len = is.current() - mark;

            is_ok = nextAndSkipWhiteSpaces<':'>(is);
            if (!is_ok)
                return false;
            mark = is.current();

            is_ok = getValueToken(is);
            if (!is_ok)
                return false;

            const char * value_name = mark;
            std::size_t value_len = is.current() - mark;

            // Append the key and value pair to StringRefList.
            entries_.append(key_name, key_len, value_name, value_len);

            skipWhiteSpaces<' '>(is);
            bool is_end;
            is_ok = checkAndSkipCrLf(is, is_end);
            if (is_end)
                return true;
            if (!is_ok)
                return false;
        } while (1);
        return is_ok;
    }

    // Parse http header
    int parseHeader(InputStream & is) {
        int ec = 0;
        bool is_ok;
        // Skip the whitespaces ahead of http header.
        skipWhiteSpaces(is);

        const char * start = is.current();
        std::size_t length = is.remain();
        // Http method characters must be upper case letters.
        if (is.get() >= 'A' && is.get() <= 'Z') {
            is_ok = parseHttpMethod(is);
            //is_ok = parseHttpMethodAndHash(is);
            if (!is_ok) {
                ec = error_code::InvalidHttpMethod;
                goto parse_error;
            }
            is_ok = nextAndSkipWhiteSpaces(is);
            if (!is_ok)
                return error_code::HttpParserError;

            is_ok = parseHttpURI(is);
            if (!is_ok)
                return error_code::HttpParserError;
            is_ok = nextAndSkipWhiteSpaces(is);
            if (!is_ok)
                return error_code::HttpParserError;

            is_ok = parseHttpVersion(is);
            if (!is_ok)
                return error_code::HttpParserError;
            is_ok = nextAndSkipWhiteSpaces_CrLf(is);
            if (!is_ok)
                return error_code::HttpParserError;

            assert(is.current() >= start);
            assert(length >= (std::size_t)(is.current() - start));
            entries_.setRef(is.current(), length - (is.current() - start));

            is_ok = parseHttpEntryList(is);
            if (!is_ok)
                return error_code::HttpParserError;
        }
        else {
            ec = error_code::InvalidHttpMethod;
        }
parse_error:
        return ec;
    }

    // Copy the input http header data.
    const char * cloneContent(const char * data, size_t len) {
        assert(data != nullptr);
        const char * content;
        if (len < kInitContentSize) {
            ::memcpy((void *)&inner_content_[0], data, len);
            inner_content_[len] = '\0';
            if (content_)
                content_ = nullptr;
            content = const_cast<const char * >(&inner_content_[0]);
            content_size_ = len;
        }
        else {
            char * new_content = new char [len + 1];
            if (new_content != nullptr) {
                ::memcpy((void *)new_content, data, len);
                new_content[len] = '\0';
                content_ = const_cast<const char *>(new_content);
                content_size_ = len;
                content = content_;
            }
            else {
                return nullptr;
            }
        }
        return content;
    }

    int parse(const char * data, size_t len) {
        int ec = 0;
        assert(data != nullptr);
        if (len == 0 || data == nullptr)
            return error_code::Succeed;

        // Copy the input http header data.
        const char * content = cloneContent(data, len);
        if (content == nullptr)
            return error_code::HttpParserError;

        // Start parse the http header.
        InputStream is(content, len);
        ec = parseHeader(is);
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
