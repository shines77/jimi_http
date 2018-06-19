
#ifndef JIMI_HTTP_PARSER_H
#define JIMI_HTTP_PARSER_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <stdint.h>
#include <assert.h>
#include <cstddef>
#include <iostream>

#include "jimi/basic/stddef.h"
#include "jimi/InputStream.h"
#include "jimi/StringRef.h"
#include "jimi/StringRefList.h"
#include "jimi/http/Common.h"
#include "jimi/http/Request.h"
#include "jimi/http/Response.h"

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

template <typename StringType = std::string, std::size_t InitContentSize = 1024>
class BasicParser {
public:
    typedef StringType      string_type;
    typedef std::uint32_t   hash_type;

    // kInitContentSize minimize value is 256.
    static const std::size_t kMinContentSize = 256;
    // kInitContentSize = max(InitContentSize, 256);
    static const std::size_t kInitContentSize = (InitContentSize > kMinContentSize)
                                                ? InitContentSize : kMinContentSize;

private:
    int32_t status_code_;
    uint32_t method_;
    Version version_;

    string_type method_str_;
    string_type uri_str_;
    string_type version_str_;

    std::size_t content_length_;
    std::size_t content_size_;
    const char * content_;
    StringRefList<64> header_fields_;
    char inner_content_[kInitContentSize];

public:
    BasicParser() : status_code_(0),
        method_(Request::UNDEFINED),
        version_(Version::HTTP_UNDEFINED),
        content_length_(0),
        content_size_(0), content_(nullptr) {
    }

    ~BasicParser() {
        if (unlikely(content_)) {
            delete[] content_;
            content_ = nullptr;
        }
    }

    void reset() {
        method_str_.clear();
        uri_str_.clear();
        version_str_.clear();
        content_size_ = 0;
        content_ = nullptr;
        header_fields_.clear();
    }

    std::size_t getEntrySize() const {
        return header_fields_.size();
    }

    uint32_t getMethod() const {
        return method_;
    }

    void setMethod(uint32_t method) {
        method_ = method;
    }

    string_type getMethodStr() const {
        return method_str_;
    }

    void setMethodStr(string_type method) {
        method_str_ = method;
    }

    uint32_t getVersion() const {
        return version_.getVersion();
    }

    void setVersion(uint32_t http_version) {
        version_.setVersion(http_version);
    }

    string_type getVersiontr() const {
        return version_str_;
    }

    void setVersionStr(string_type version) {
        version_str_ = version;
    }

    string_type getURI() const {
        return uri_str_;
    }

    void setURI(string_type uri) {
        uri_str_ = uri;
    }

    void next(InputStream & is) {
        is.next();
    }

    void moveTo(InputStream & is, int offset) {
        is.moveTo(offset);
    }

    bool hasNextChar(InputStream & is) {
        return is.hasNextChar();
    }

#if 1
    void skipWhiteSpaces(InputStream & is) {
        assert(is.current() != nullptr);
        while (is.hasNext() && (is.get() == ' ')) {
            is.next();
        }
    }
#else
    void skipWhiteSpaces(InputStream & is) {
        assert(is.current() != nullptr);
        while (is.hasNext() && (is.get() == ' ' || is.get() == '\t')) {
            is.next();
        }
    }
#endif

    bool skipCrLf(InputStream & is) {
        assert(is.current() != nullptr);
        while (is.hasNext()) {
            if (is.get() == '\r' || is.get() == '\n')
                is.next();
            else
                break;
        }
        return is.hasNext();
    }

    void nextAndSkipCrLf(InputStream & is) {
        if (is.remain() > 2) {
            moveTo(is, 2);
        }
    }

    bool skipCrLfAndWhiteSpaces(InputStream & is) {
        while (is.hasNext()) {
            if (is.get() == '\r' || is.get() == ' ' || is.get() == '\n' || is.get() == '\t')
                is.next();
            else
                break;
        }
        return is.hasNext();
    }

    template <char delimiter>
    bool findToken(InputStream & is) {
        assert(is.current() != nullptr);
        while (is.hasNext() && (is.get() != delimiter && is.get() != ' ' && !is.isNullChar())) {
            is.next();
        }
        return is.hasNext();
    }

    template <char delimiter>
    bool findTokenAndHash(InputStream & is, hash_type & hash) {
        static const hash_type kSeedTime31 = 31U;
        hash = 0;
        assert(is.current() != nullptr);
        while (is.hasNext() && (is.get() != delimiter && is.get() != ' ' && !is.isNullChar())) {
            hash += static_cast<hash_type>(is.get()) * kSeedTime31;
            is.next();
        }
        return is.hasNext();
    }

    bool findCrLfToken(InputStream & is) {
        assert(is.current() != nullptr);
        while (likely(is.hasNext())) {
            if (unlikely(is.get() == '\r')) {
                if (likely(is.peek(1) == '\n') && likely(is.hasNext(1)))
                    return true;
            }
            if (likely(!is.isNullChar()))
                is.next();
            else
                break;
        }
        return is.hasNext();
    }

    bool findFieldName(InputStream & is) {
        assert(is.current() != nullptr);
        while (is.hasNext() && (is.get() != ':' && is.get() != ' ' && !is.isNullChar())) {
            is.next();
        }
        return is.hasNext();
    }

    bool findFieldValue(InputStream & is) {
        assert(is.current() != nullptr);
        while (is.hasNext() && (is.get() != '\r' && !is.isNullChar())) {
            is.next();
        }
        return is.hasNext();
    }

    bool checkAndSkipCrLf(InputStream & is, bool & is_end) {
        assert(is.current() != nullptr);
        
        is_end = false;
        if (likely(is.remain() >= 4)) {
            // If the remain length is more than or equal 4 bytes, needn't to check if is hasNext().
            if (likely(is.get() == '\r')) {
                if (likely(is.peek(2) != '\r')) {
                    if (likely(is.peek(1) == '\n')) {
                        is.moveTo(2);       // "\r\nX", In most cases, it will be walk to this path.
                        return true;
                    }
                    return false;
                }
                else {
                    if (likely(is.peek(1) == '\n') && likely(is.peek(3) == '\n')) {
                        is.moveTo(4);       // "\r\n\r\n", It's the end of the http header.
                        is_end = true;
                        return true;
                    }
                }
            }
            return false;
        }
        else {
            // If the remain length is less than 4 bytes, we need to check if is hasNext().
            //if (unlikely(!is.hasNext()))
            //    return false;
            if (likely(is.get() == '\r')) {
                if (likely(is.hasNext(1)) && likely(is.peek(1) == '\n')) {
                    is.moveTo(2);       // "\r\nX", In most cases, it will be walk to this path.
                    return true;
                }
            }
        }
        return false;
    }

    bool checkAndSkipCrLf_Midium(InputStream & is, bool & is_end) {
        assert(is.current() != nullptr);
        is_end = false;
scan_restart:
        if (likely(is.remain() >= 4)) {
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
                    goto scan_restart;
                }
                else if (is.get() == '\0') {
                    is_end = true;
                    return false;
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
                    return false;
                }
                else {
                    break;
                }
            } while (1);
        }
        return false;
    }

    bool checkAndSkipCrLf_Heavy(InputStream & is, bool & is_end) {
        assert(is.current() != nullptr);
        is_end = false;
        if (likely(is.remain() >= 4)) {
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
                    return false;
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
                    return false;
                }
                else {
                    break;
                }
            } while (1);
            return false;
        }
    }

    bool parseMethod(InputStream & is) {
        if (!method_str_.empty())
            return false;
        const char * mark = is.current();
        bool is_ok = findToken<' '>(is);
        if (likely(is_ok)) {
            assert(is.current() != nullptr);
            assert(is.current() >= mark);
            std::ptrdiff_t len = is.current() - mark;
            method_str_.assign(mark, len);
            if (unlikely(len <= 0))
                return false;
        }
        return is_ok;
    }

    bool parseMethodAndHash(InputStream & is) {
        if (!method_str_.empty())
            return false;
        hash_type hash;
        const char * mark = is.current();
        bool is_ok = findTokenAndHash<' '>(is, hash);
        if (likely(is_ok)) {
            assert(is.current() != nullptr);
            assert(is.current() >= mark);
            std::ptrdiff_t len = is.current() - mark;
            method_str_.assign(mark, len);
            if (unlikely(len <= 0))
                return false;
        }
        return is_ok;
    }

    bool parseURI(InputStream & is) {
        if (!uri_str_.empty())
            return false;
        const char * mark = is.current();
        bool is_ok = findToken<' '>(is);
        if (likely(is_ok)) {
            assert(is.current() != nullptr);
            assert(is.current() >= mark);
            std::ptrdiff_t len = is.current() - mark;
            uri_str_.assign(mark, len);
            if (unlikely(len <= 0))
                return false;
        }
        return is_ok;
    }

    bool parseVersion(InputStream & is) {
        if (!version_str_.empty())
            return false;
        const char * mark = is.current();
        bool is_ok = findCrLfToken(is);
        if (likely(is_ok)) {
            assert(is.current() != nullptr);
            assert(is.current() >= mark);
            static const std::ptrdiff_t kLenHTTPVersion = sizeof("HTTP/1.1") - 1;
            std::ptrdiff_t len = is.current() - mark;
            if (likely(len >= kLenHTTPVersion)) {
                version_str_.assign(mark, len);
                return true;
            }
            else
                return false;
        }
        return is_ok;
    }

    bool parseHeaderFields(InputStream & is) {
        do {
            // Skip the whitespaces ahead of every entry.
            //skipWhiteSpaces(is);

            const char * field_name = is.current();
            bool is_ok = findFieldName(is);

            std::ptrdiff_t name_len = is.current() - field_name;
            if (unlikely(!is_ok || (name_len <= 0)))
                return false;

            next(is);
            skipWhiteSpaces(is);

            const char * field_value = is.current();
            is_ok = findFieldValue(is);

            std::ptrdiff_t value_len = is.current() - field_value;
            if (unlikely(!is_ok) || unlikely(value_len <= 0))
                return false;

            // Append the field-name and field-value pair to StringRefList.
            header_fields_.append(field_name, name_len, field_value, value_len);

            //skipWhiteSpaces(is);

            bool is_end;
            is_ok = checkAndSkipCrLf(is, is_end);
            if (unlikely(is_end))
                return true;
            if (unlikely(!is_ok))
                return false;
        } while (1);
        return false;
    }

    // Parse request http header
    int parseRequestHeader(InputStream & is) {
        int ec = 0;
        bool is_ok;
        // Skip the whitespaces ahead of request http header.
        //skipWhiteSpaces(is);

        const char * start = is.current();
        std::size_t length = is.remain();
        // Http method characters must be upper case letters.
        if (likely(is.get() >= 'A' && is.get() <= 'Z')) {
            is_ok = parseMethod(is);
            //is_ok = parseHttpMethodAndHash(is);
            if (unlikely(!is_ok))
                return error_code::InvalidHttpMethod;

            next(is);
            skipWhiteSpaces(is);

            is_ok = parseURI(is);
            if (unlikely(!is_ok))
                return error_code::HttpParserError;

            next(is);
            skipWhiteSpaces(is);

            is_ok = parseVersion(is);
            if (unlikely(!is_ok))
                return error_code::HttpParserError;

            // Skip the CrLf, move the cursor 2 bytes.
            assert(is.remain() >= 2);
            moveTo(is, 2);
 
            assert(is.current() >= start);
            assert(length >= (std::size_t)(is.current() - start));
            header_fields_.setRef(is.current(), length - (is.current() - start));

            is_ok = parseHeaderFields(is);
            if (unlikely(!is_ok))
                return error_code::HttpParserError;
        }
        else {
            ec = error_code::InvalidHttpMethod;
        }
        return ec;
    }

    // Copy the input http header data.
    const char * copyContent(const char * data, size_t len) {
        assert(data != nullptr);
        const char * content;
        if (likely(len < kInitContentSize)) {
            ::memcpy((void *)&inner_content_[0], data, len);
            inner_content_[len] = '\0';
            if (unlikely(content_))
                content_ = nullptr;
            content = const_cast<const char * >(&inner_content_[0]);
            content_size_ = len;
        }
        else {
            char * new_content = new char [len + 1];
            if (likely(new_content != nullptr)) {
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

    int parseRequest(const char * data, size_t len) {
        int ec = 0;
        assert(data != nullptr);
        if (unlikely(len == 0 || data == nullptr))
            return error_code::Succeed;

        // Copy the input http header data.
        const char * content = copyContent(data, len);
        if (unlikely(content == nullptr))
            return error_code::HttpParserError;

        // Start parse the request http header.
        InputStream is(content, len);
        ec = parseRequestHeader(is);
        return ec;
    }

    int parseRequest(const std::string & data) {
        return parseRequest(data.data(), data.size());
    }

    void displayFields() {
        std::cout << "Http entries: (length = " << header_fields_.ref.size() << " bytes)" << std::endl << std::endl;
        std::cout << header_fields_.ref.c_str() << std::endl;

        std::cout << "Http entries size: " << header_fields_.size() << std::endl << std::endl;
        std::size_t data_len = header_fields_.ref.size();
        for (std::size_t i = 0; i < header_fields_.size(); ++i) {
            if ((header_fields_.items[i].key.offset <= data_len)
                && ((header_fields_.items[i].key.offset + header_fields_.items[i].key.length) < data_len)
                && (header_fields_.items[i].value.offset <= data_len)
                && ((header_fields_.items[i].value.offset + header_fields_.items[i].value.length) < data_len)) {
                std::string key(header_fields_.ref.data() + header_fields_.items[i].key.offset, header_fields_.items[i].key.length);
                std::string value(header_fields_.ref.data() + header_fields_.items[i].value.offset, header_fields_.items[i].value.length);
                std::cout << "key = " << key.c_str() << ", value = " << value.c_str() << std::endl;
            }
            else {
                std::cout << "Error entry." << std::endl;
            }
        }
        std::cout << std::endl;
    }
};

template <std::size_t InitContentSize = 1024>
using Parser = BasicParser<std::string>;

template <std::size_t InitContentSize = 1024>
using ParserRef = BasicParser<StringRef>;

} // namespace http
} // namespace jimi

#endif // JIMI_HTTP_PARSER_H
