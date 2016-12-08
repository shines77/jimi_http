
#ifndef JIMI_HTTP_COMMON_H
#define JIMI_HTTP_COMMON_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <stdint.h>

namespace jimi {
namespace http {

class Version {
public:
    enum version {
        HTTP_UNDEFINED = 0,
        HTTP_0_9 = 0x00000009,
        HTTP_1_0 = 0x00010000,
        HTTP_1_1 = 0x00010001,
        HTTP_2_0 = 0x00020000,
        HTTP_2_X = 0x00020001,
    };

    union version_t {
        struct {
            uint16_t major_;
            uint16_t minor_;
        };
        uint32_t value;

        version_t() {}
        version_t(uint32_t version) : value(version) {}
        version_t(uint16_t major, uint16_t minor) : major_(major), minor_(minor) {}
    };

private:
    version_t version_;

public:
    Version() : version_(0) {}
    Version(uint32_t version) : version_(version) {}
    Version(uint16_t major, uint16_t minor) : version_(major, minor) {}
    Version(const Version & src) : version_(src.getVersion()) {}
    ~Version() {}

    Version & operator = (uint32_t version) {
        version_.value = version;
        return (*this);
    }

    Version & operator = (const Version & rhs) {
        version_.value = rhs.getVersion();
        return (*this);
    }

    static uint32_t makeVersion(uint32_t major, uint32_t minor) {
        version_t version(major, minor);
        return version.value;
    }

    static uint16_t calcMajor(uint32_t http_version) {
        version_t version(http_version);
        return version.major_;
    }

    static uint16_t calcMinor(uint32_t http_version) {
        version_t version(http_version);
        return version.minor_;
    }

    uint16_t getMajor() const {
        return version_.major_;
    }

    uint16_t getMinor() const {
        return version_.minor_;
    }

    uint32_t getVersion() const {
        return version_.value;
    }

    void setMajor(uint16_t major) {
        version_.major_ = major;
    }

    void setMinor(uint16_t minor) {
        version_.minor_ = minor;
    }

    void setVersion(uint32_t version) {
        version_.value = version;
    }

    bool operator == (const Version & rhs) {
        return (version_.value == rhs.getVersion());
    }

    bool operator != (const Version & rhs) {
        return (version_.value != rhs.getVersion());
    }

    bool operator > (const Version & rhs) {
        return (version_.value > rhs.getVersion());
    }

    bool operator >= (const Version & rhs) {
        return (version_.value >= rhs.getVersion());
    }

    bool operator < (const Version & rhs) {
        return (version_.value < rhs.getVersion());
    }

    bool operator <= (const Version & rhs) {
        return (version_.value <= rhs.getVersion());
    }
};

} // namespace http
} // namespace jimi

#endif // !JIMI_HTTP_COMMON_H