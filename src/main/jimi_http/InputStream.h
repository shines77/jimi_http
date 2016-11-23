
#ifndef JIMI_HTTP_INPUTSTREAM_H
#define JIMI_HTTP_INPUTSTREAM_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <stdint.h>
#include <string.h>
#include <memory.h>
#include <assert.h>
#include <cstddef>

namespace jimi {
namespace http {

template <typename CharT>
class BasicInputStream {
public:
    typedef CharT           char_type;
    typedef std::size_t     size_type;
    typedef std::ptrdiff_t  ssize_type;

private:
    char_type *         current_;
    const char_type *   data_;
    const char_type *   end_;
    size_type           size_;
    size_type           capacity_;

public:
    BasicInputStream(const char * data = nullptr) : current_(const_cast<char_type *>(data)),
        data_(data), end_((data != nullptr) ? (data + ::strlen(data)) : nullptr) {
    }
    BasicInputStream(const char * data, std::size_t len) : current_(const_cast<char_type *>(data)),
        data_(data), end_((data != nullptr) ? (data + len) : nullptr) {
    }
    ~BasicInputStream() {}

    size_type size() const {
        return (end_ - data_);
    }

    bool is_valid() const {
        return (data_ != nullptr);
    }

    bool is_eof() const {
        return (current_ == end_);
    }

    bool is_underflow() const {
        return (current_ < data_);
    }

    bool is_overflow() const {
        return (current_ >= end_);
    }

    bool is_legal() const {
        return (current_ >= data_ && current_ < end_);
    }

    bool is_nullchar() const {
        return (get() == '\0');
    }

    bool is_nullchar(int offset) const {
        return (peek(offset) == '\0');
    }

    bool is_empty() const {
        return (size() == 0);
    }

    bool hasNext() const {
        return (!is_overflow());
    }

    bool hasNext(int offset) const {
        return ((current_ + offset) >= end_);
    }

    bool hasNextChar() const {
        return (hasNext() && !is_nullchar());
    }

    bool hasNextChar(int offset) const {
        return (hasNext(offset) && !is_nullchar(offset));
    }

    char_type * current() const {
        return current_;
    }

    const char_type * data() const {
        return data_;
    }

    const char_type * end() const {
        return end_;
    }

    ssize_type offset() const {
        return (current_ - data_);
    }

    ssize_type remain() const {
        return (end_ - current_);
    }

    char_type get() const {
        assert(current_ != nullptr);
        return *current_;
    }

    char_type peek(int offset) const {
        assert((current_ + offset) != nullptr);
        return *(current_ + offset);
    }

    void put(char_type ch) {
        assert(current_ != nullptr);
        *current_ = ch;
    }

    void put(char_type ch, int offset) {
        assert((current_ + offset) != nullptr);
        *(current_ + offset) = ch;
    }

    void next() {
        current_++;
    }

    void moveTo(int offset) {
        current_ += offset;
    }

    char_type * nextAndGet() const {
        next();
        return get();
    }

    void putAndNext(char_type ch) {
        put(ch);
        next();
    }
};

typedef BasicInputStream<char>      InputStreamA;
typedef BasicInputStream<wchar_t>   InputStreamW;
typedef InputStreamA                InputStream;

} // namespace http
} // namespace jimi

#endif // !JIMI_HTTP_INPUTSTREAM_H
