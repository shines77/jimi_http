
#ifndef JIMI_HTTP_COMMON_H
#define JIMI_HTTP_COMMON_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <stdint.h>

namespace jimi {
namespace http {

class HttpVersion {
public:
    enum version
    {
        HTTP_UNDEFINED = 0,
        HTTP_0_9 = 0x00000009,
        HTTP_1_0 = 0x00010000,
        HTTP_1_1 = 0x00010001,
        HTTP_2_0 = 0x00020000,
        HTTP_2_X = 0x00020001,
    };

    union http_version_t
    {
        struct info_t
        {
            uint16_t major_;
            uint16_t minor_;

            info_t(uint16_t _major, uint16_t _minor) : major_(_major), minor_(_minor) {}
        } v;
        uint32_t value;

        http_version_t() {}
        http_version_t(uint32_t version) : value(version) {}
        http_version_t(uint16_t _major, uint16_t _minor) : v(_major, _minor) {}
    };

private:
    http_version_t version_;

public:
    HttpVersion() : version_(0) {}
    HttpVersion(uint32_t version) : version_(version) {}
    HttpVersion(uint16_t major, uint16_t minor) : version_(major, minor) {}
    HttpVersion(const HttpVersion & src) : version_(src.getVersion()) {}
    ~HttpVersion() {}

    HttpVersion & operator = (uint32_t version) {
        version_.value = version;
        return (*this);
    }

    HttpVersion & operator = (const HttpVersion & rhs) {
        version_.value = rhs.getVersion();
        return (*this);
    }

    static uint32_t makeVersion(uint32_t major, uint32_t minor) {
        http_version_t version(major, minor);
        return version.value;
    }

    static uint16_t calcMajor(uint32_t http_version) {
        http_version_t version(http_version);
        return version.v.major_;
    }

    static uint16_t calcMinor(uint32_t http_version) {
        http_version_t version(http_version);
        return version.v.minor_;
    }

    uint16_t getMajor() const {
        return version_.v.major_;
    }

    uint16_t getMinor() const {
        return version_.v.minor_;
    }

    uint32_t getVersion() const {
        return version_.value;
    }

    void setMajor(uint16_t major) {
        version_.v.major_ = major;
    }

    void setMinor(uint16_t minor) {
        version_.v.minor_ = minor;
    }

    void setVersion(uint32_t version) {
        version_.value = version;
    }

    bool operator == (const HttpVersion & rhs) {
        return (version_.value == rhs.getVersion());
    }

    bool operator != (const HttpVersion & rhs) {
        return (version_.value != rhs.getVersion());
    }

    bool operator > (const HttpVersion & rhs) {
        return (version_.value > rhs.getVersion());
    }

    bool operator >= (const HttpVersion & rhs) {
        return (version_.value >= rhs.getVersion());
    }

    bool operator < (const HttpVersion & rhs) {
        return (version_.value < rhs.getVersion());
    }

    bool operator <= (const HttpVersion & rhs) {
        return (version_.value <= rhs.getVersion());
    }
};

} // namespace http
} // namespace jimi

#endif // !JIMI_HTTP_COMMON_H
