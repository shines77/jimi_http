
#ifndef JIMI_HTTP_COMMON_H
#define JIMI_HTTP_COMMON_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <stdint.h>

namespace jimi {
namespace http {

union version_t {
    struct ver {
        uint16_t major;
        uint16_t minor;

        ver(uint16_t _major, uint16_t _minor) : major(_major), minor(_minor) {}
        ~ver() {}
    } v;
    uint32_t value;

    version_t(uint32_t version = 0) : value(version) {}
    version_t(uint16_t major, uint16_t minor) : v(major, minor) {}
    version_t(const version_t & src) : value(src.value) {}
    ~version_t() {}

    version_t & operator = (uint32_t version) {
        this->value = version;
        return (*this);
    }

    version_t & operator = (const version_t & rhs) {
        this->value = rhs.value;
        return (*this);
    }
};

class Version {
public:
    enum Type {
        UNKNOWN = 0,
        HTTP_0_9 = 0x00000009,
        HTTP_1_0 = 0x00010000,
        HTTP_1_1 = 0x00010001,
        HTTP_2_0 = 0x00020000,
        HTTP_2_X = 0x00020001,
    };

private:
    version_t version_;

public:
    Version(uint32_t version = Type::UNKNOWN) : version_(version) {}
    Version(uint16_t major, uint16_t minor) : version_(major, minor) {}
    Version(const version_t & src) : version_(src.value) {}
    Version(const Version & src) : version_(src.getVersion()) {}
    ~Version() {}

    Version & operator = (uint32_t version) {
        version_.value = version;
        return (*this);
    }

    Version & operator = (const version_t & rhs) {
        version_.value = rhs.value;
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

    static uint16_t getMajor(uint32_t http_version) {
        version_t version(http_version);
        return version.v.major;
    }

    static uint16_t getMinor(uint32_t http_version) {
        version_t version(http_version);
        return version.v.minor;
    }

    uint16_t getMajor() const {
        return version_.v.major;
    }

    uint16_t getMinor() const {
        return version_.v.minor;
    }

    uint32_t getVersion() const {
        return version_.value;
    }

    void setMajor(uint16_t major) {
        version_.v.major = major;
    }

    void setMinor(uint16_t minor) {
        version_.v.minor = minor;
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

#endif // JIMI_HTTP_COMMON_H
