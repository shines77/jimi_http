
#ifndef JIMI_HTTP_STRINGREF_H
#define JIMI_HTTP_STRINGREF_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <stdint.h>
#include <cstddef>
#include <string>

namespace jimi {
namespace http {

namespace detail {

    //////////////////////////////////////////
    // function strlen<T>()
    //////////////////////////////////////////

    template <typename CharType>
    std::size_t strlen(CharType * str) {
        return 0;
    }

    template <>
    std::size_t strlen(const char * str) {
        return ::strlen(str);
    }

    template <>
    std::size_t strlen(const uint16_t * str) {
        return ::wcslen((const wchar_t *)str);
    }

    template <>
    std::size_t strlen(const wchar_t * str) {
        return ::wcslen(str);
    }

    //////////////////////////////////////////
}

template <typename CharType>
class BasicStringRef {
public:
    typedef CharType char_type;
    typedef std::size_t size_type;
    typedef std::basic_string<char_type> std_string;

private:
    const char_type * data_;
    size_type size_;

public:
    BasicStringRef() : data_(nullptr), size_(0) {}
    BasicStringRef(const char_type * data) : data_(data), size_(detail::strlen(data)) {}
    BasicStringRef(const char_type * data, size_type size) : data_(data), size_(size) {}
    template <size_type N>
    BasicStringRef(char_type (&src)[N]) : data_(src), size_(N) {}
    BasicStringRef(const std_string & src) : data_(src.data()), size_(src.size()) {}
    BasicStringRef(std_string && src) : data_(src.data()), size_(src.size()) {}
    BasicStringRef & operator = (const std_string & rhs) {
        this->data_ = rhs.data();
        this->size_ = rhs.size();
        return *this;
    }
    BasicStringRef(const BasicStringRef & src) : data_(src.data()), size_(src.size()) {}
    BasicStringRef(BasicStringRef && src) : data_(src.data()), size_(src.size()) {
        src.reset();
    }
    BasicStringRef & operator = (const BasicStringRef & rhs) {
        this->data_ = rhs.data();
        this->size_ = rhs.size();
        return *this;
    }
    ~BasicStringRef() {}

    const char_type * data() const { return data_; }
    size_type size() const  { return size_; }

    const char_type * c_str() const { return (data() != nullptr) ? data() : ""; }
    size_type length() const { return size(); }

    bool empty() const { return (size() == 0); }

    void reset() {
        data_ = nullptr;
        size_ = 0;
    }

    void clear() {
        data_ = "";
        size_ = 0;
    }

    void assign(const char_type * data) {
        data_ = data;
        size_ = ::strlen(data);
    }

    void assign(const char_type * data, size_type size) {
        data_ = data;
        size_ = size;
    }

    void assign(const std_string & src) {
        data_ = src.data();
        size_ = src.size();
    }

    template <size_type N>
    void assign(char_type (&src)[N]) {
        data_ = src;
        size_ = N;
    }

    std::string toString() const {
        return std::string(data_, size_);
    }
};

template <typename CharType>
class BasicStringRefHelper {
public:
    typedef CharType char_type;
    typedef BasicStringRef<char_type> string_ref;

private:
    string_ref & str_;
    char_type save_char_;
    bool truncate_;

public:
    BasicStringRefHelper() : str_(string_ref("")), save_char_('\0'), truncate_(false) {}
    ~BasicStringRefHelper() {}

    bool attach(const string_ref & str) {
        // If the string reference don't recover the truncated char,
        // don't accept the new attach.
        if (!truncate_) {
            str_ = str;
        }
        return (!truncate_);
    }

    void truncate() {
        if (!truncate_) {
            char_type * first = (char_type *)str_.data();
            char_type * last = first + str_.size();
            assert(last != nullptr);
            save_char_ = *last;
            *last = static_cast<char_type>('\0');
            truncate_ = true;
        }
    }

    void recover() {
        if (truncate_) {
            char_type * first = (char_type *)str_.data();
            char_type * last = first + str_.size();
            assert(last != nullptr);
            *last = save_char_;
            truncate_ = false;
        }
    }
};

typedef BasicStringRef<char>        StringRefA;
typedef BasicStringRef<wchar_t>     StringRefW;
typedef BasicStringRef<char>        StringRef;

typedef BasicStringRefHelper<char>      StringRefHelperA;
typedef BasicStringRefHelper<wchar_t>   StringRefHelperW;
typedef BasicStringRefHelper<char>      StringRefHelper;

} // namespace http
} // namespace jimi

#endif // !JIMI_HTTP_STRINGREF_H
