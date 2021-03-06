
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
#include "jimi/http/Version.h"
#include "jimi/http/Request.h"
#include "jimi/http/Response.h"

using namespace std;

/////////////////////////////////////////////////////////////////////////////////////////
//
// HTTP response smuggling
//
//   https://www.mozilla.org/en-US/security/advisories/mfsa2006-33/
//
// HTTP Response Smuggling
//
//   https://www.whitehatsec.com/glossary/content/http-response-smuggling
//
// HTTP Response Splitting
//
//   https://www.whitehatsec.com/glossary/content/http-response-splitting
//
// Testing for HTTP Splitting/Smuggling (OTG-INPVAL-016)
//
//   https://www.owasp.org/index.php/Testing_for_HTTP_Splitting/Smuggling_(OTG-INPVAL-016)
//   http://netsecurity.51cto.com/art/201001/177371.htm (Chinese)
//
// Content injection spoofing with a space before colon in HTTP header
//
//   https://bugzilla.mozilla.org/show_bug.cgi?id=329746
//
// Difference between HTTP Splitting AND HTTP Smuggling?
//
//   https://stackoverflow.com/questions/28580568/difference-between-http-splitting-and-http-smuggling
//
/////////////////////////////////////////////////////////////////////////////////////////

namespace jimi {
namespace http {

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
        method_(Method::UNKNOWN),
        version_(Version::UNKNOWN),
        content_length_(0),
        content_size_(0), content_(nullptr) {
    }

    ~BasicParser() {
        if (unlikely(content_ != nullptr)) {
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

    std::size_t getFieldSize() const {
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

    string_type getVersionStr() const {
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
        if (likely(is.get() != ' '))
            return;
        while (likely(is.hasNext())) {
            if (likely(is.get() == ' '))
                is.next();
            else
                break;
        }
    }
#elif 0
    void skipWhiteSpaces(InputStream & is) {
        assert(is.current() != nullptr);
        while (likely(is.hasNext())) {
            if (likely(is.get() == ' '))
                is.next();
            else
                break;
        }
    }
#else
    void skipWhiteSpaces(InputStream & is) {
        assert(is.current() != nullptr);
        while (likely(is.hasNext())) {
            if (likely(is.get() == ' ' || is.get() == '\t'))
                is.next();
            else
                break;
        }
    }
#endif

    bool skipCrLf(InputStream & is) {
        assert(is.current() != nullptr);
        while (likely(is.hasNext())) {
            if (likely(is.get() == '\r')) {
                is.next();
                if (likely(is.get() == '\n'))
                    is.next();
                else
                    break;
            }
            else {
                break;
            }
        }
        return is.hasNext();
    }

    void nextAndSkipCrLf(InputStream & is) {
        if (likely(is.remain() > 2)) {
            moveTo(is, 2);
        }
    }

    bool skipCrLfAndWhiteSpaces(InputStream & is) {
        while (likely(is.hasNext())) {
            if (likely(is.get() == '\r' || is.get() == ' ' || is.get() == '\n' || is.get() == '\t'))
                is.next();
            else
                break;
        }
        return is.hasNext();
    }

    template <char delimiter>
    bool findToken(InputStream & is) {
        assert(is.current() != nullptr);
        while (likely(is.hasNext())) {
            if (likely(is.get() != delimiter && is.get() != ' ' && !is.isNullChar()))
                is.next();
            else
                break;
        }
        return is.hasNext();
    }

    template <char delimiter>
    bool findTokenAndHash(InputStream & is, hash_type & hash) {
        static const hash_type kSeedTime31 = 31U;
        hash = 0;
        assert(is.current() != nullptr);
        while (likely(is.hasNext())) {
            if (likely(is.get() != delimiter && is.get() != ' ' && !is.isNullChar())) {
                hash += static_cast<hash_type>(is.get()) * kSeedTime31;
                is.next();
            }
            else {
                break;
            }
        }
        return is.hasNext();
    }

    bool findCrLfToken(InputStream & is) {
        assert(is.current() != nullptr);
        while (likely(is.hasNext())) {
            if (likely(is.get() == '\r')) {
                if (likely((is.peek(1) == '\n') && is.hasNext(1)))
                    return true;
                else
                    is.next();
            }
            else {
                if (likely(!is.isNullChar()))
                    is.next();
                else
                    break;
            }
        }
        return is.hasNext();
    }

    bool findFieldKey(InputStream & is) {
        assert(is.current() != nullptr);
        while (likely(is.hasNext())) {
            if (likely(is.get() != ':' && !is.isNullChar()))
                is.next();
            else
                break;
        }
        return is.hasNext();
    }

    bool findFieldValue(InputStream & is) {
        assert(is.current() != nullptr);
        while (likely(is.hasNext())) {
            if (likely(is.get() != '\r' && !is.isNullChar()))
                is.next();
            else
                break;
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
                    if (likely((is.peek(1) == '\n') && (is.peek(3) == '\n'))) {
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
                if (likely(is.hasNext(1) && (is.peek(1) == '\n'))) {
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
        //if (likely(!method_str_.empty()))
        //    return false;
        const char * mark = is.current();
        bool is_ok = findToken<' '>(is);
        if (likely(is_ok)) {
            assert(is.current() != nullptr);
            assert(is.current() >= mark);
            std::ptrdiff_t len = is.current() - mark;
            assert(len > 0);
            method_str_.assign(mark, len);
        }
        return is_ok;
    }

    bool parseMethodAndHash(InputStream & is) {
        //if (likely(!method_str_.empty()))
        //    return false;
        hash_type hash;
        const char * mark = is.current();
        bool is_ok = findTokenAndHash<' '>(is, hash);
        if (likely(is_ok)) {
            assert(is.current() != nullptr);
            assert(is.current() >= mark);
            std::ptrdiff_t len = is.current() - mark;
            assert(len > 0);
            method_str_.assign(mark, len);
        }
        return is_ok;
    }

    bool parseURI(InputStream & is) {
        //if (likely(!uri_str_.empty()))
        //   return false;
        const char * mark = is.current();
        bool is_ok = findToken<' '>(is);
        if (likely(is_ok)) {
            assert(is.current() != nullptr);
            assert(is.current() >= mark);
            std::ptrdiff_t len = is.current() - mark;
            assert(len > 0);
            uri_str_.assign(mark, len);
        }
        return is_ok;
    }

    bool parseVersion(InputStream & is) {
        //if (likely(!version_str_.empty()))
        //    return false;
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
            else {
                return false;
            }
        }
        return is_ok;
    }

    bool parseHeaderFields(InputStream & is) {
        do {
            // Skip the whitespaces ahead of every entry.
            //skipWhiteSpaces(is);

            const char * field_key = is.current();
            bool is_ok = findFieldKey(is);

            std::ptrdiff_t key_len = is.current() - field_key;
            if (likely(is_ok && (key_len > 0))) {
                next(is);
                skipWhiteSpaces(is);

                const char * field_value = is.current();
                is_ok = findFieldValue(is);

                std::ptrdiff_t value_len = is.current() - field_value;
                if (likely(is_ok && (value_len > 0))) {
                    // Append the field-name and field-value pair to StringRefList.
                    header_fields_.append(field_key, key_len, field_value, value_len);

                    //skipWhiteSpaces(is);

                    bool is_end;
                    is_ok = checkAndSkipCrLf(is, is_end);
                    if (likely(!is_end)) {
                        if (likely(is_ok))
                            continue;
                        else
                            break;
                    }
                    else {
                        return true;
                    }
                }
                else {
                    break;
                }
            }
            else {
                break;
            }
        } while (1);
        return false;
    }

    // Parse request http header
    int parseRequestHeader(InputStream & is) {
        int ec = error_code::Succeed;
        bool is_ok;
        // Skip the whitespaces ahead of request http header.
        //skipWhiteSpaces(is);

        const char * start = is.current();
        std::size_t length = is.remain();
        // Http method characters must be upper case letters.
        if (likely(is.get() >= 'A' && is.get() <= 'Z')) {
            is_ok = parseMethod(is);
            if (likely(is_ok)) {
                next(is);
                skipWhiteSpaces(is);

                is_ok = parseURI(is);
                if (likely(is_ok)) {
                    next(is);
                    skipWhiteSpaces(is);

                    is_ok = parseVersion(is);
                    if (likely(is_ok)) {
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
                        ec = error_code::HttpParserError;
                    }
                }
                else {
                    ec = error_code::HttpParserError;
                }
            }
            else {
                ec = error_code::InvalidHttpMethod;
            }
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
            ::memcpy((void *)&inner_content_[0], data, len * sizeof(char));
            inner_content_[len] = '\0';
            content_ = nullptr;
            content = const_cast<const char * >(&inner_content_[0]);
            content_size_ = len;
        }
        else {
            char * new_content = new char [len + 1];
            if (likely(new_content != nullptr)) {
                ::memcpy((void *)new_content, data, len * sizeof(char));
                new_content[len] = '\0';
                content_ = const_cast<const char *>(new_content);
                content_size_ = len;
                content = content_;
            }
            else {
                content = nullptr;
            }
        }
        return content;
    }

    int parseRequest(const char * data, size_t len) {
        int ec = 0;
        assert(data != nullptr);
        if (likely(len != 0)) {
            // Copy the input http header data.
            const char * content = data;    //copyContent(data, len);
            if (likely(content != nullptr)) {
                // Start parse the request http header.
                InputStream is(content, len);
                ec = parseRequestHeader(is);
                return ec;
            }
            else {
                return error_code::HttpParserError;
            }
        }
        else {
            return error_code::Succeed;
        }
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
