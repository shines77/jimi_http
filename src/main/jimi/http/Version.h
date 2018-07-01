
#ifndef JIMI_HTTP_VERSION_H
#define JIMI_HTTP_VERSION_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "jimi/basic/stddef.h"
#include "jimi/basic/stdint.h"

namespace jimi {
namespace http {

union version_t {
    struct {
        uint16_t major_;
        uint16_t minor_;
    };
    uint32_t value;

    version_t(uint32_t version = 0) : value(version) {}
    version_t(uint16_t major, uint16_t minor) : major_(major), minor_(minor) {}
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
        this->version_.value = version;
        return (*this);
    }

    Version & operator = (const version_t & rhs) {
        this->version_.value = rhs.value;
        return (*this);
    }

    Version & operator = (const Version & rhs) {
        this->version_.value = rhs.getVersion();
        return (*this);
    }

    operator uint32_t () {
        return this->getVersion();
    }

    operator version_t () {
        return this->version_;
    }

    static uint16_t getMajor(uint32_t http_version) {
        version_t version(http_version);
        return version.major_;
    }

    static uint16_t getMinor(uint32_t http_version) {
        version_t version(http_version);
        return version.minor_;
    }

    static uint32_t makeVersion(uint16_t major, uint16_t minor) {
        version_t version(major, minor);
        return version.value;
    }

    uint16_t getMajor() const {
        return this->version_.major_;
    }

    uint16_t getMinor() const {
        return this->version_.minor_;
    }

    uint32_t getVersion() const {
        return this->version_.value;
    }

    void setMajor(uint16_t major) {
        this->version_.major_ = major;
    }

    void setMinor(uint16_t minor) {
        this->version_.minor_ = minor;
    }

    void setVersion(uint32_t version) {
        this->version_.value = version;
    }

    bool operator == (uint32_t rhs) {
        return (this->version_.value == rhs);
    }

    bool operator != (uint32_t rhs) {
        return (this->version_.value != rhs);
    }

    bool operator == (const Version & rhs) {
        return (this->version_.value == rhs.getVersion());
    }

    bool operator != (const Version & rhs) {
        return (this->version_.value != rhs.getVersion());
    }

    bool operator > (const Version & rhs) {
        return (this->version_.value > rhs.getVersion());
    }

    bool operator >= (const Version & rhs) {
        return (this->version_.value >= rhs.getVersion());
    }

    bool operator < (const Version & rhs) {
        return (this->version_.value < rhs.getVersion());
    }

    bool operator <= (const Version & rhs) {
        return (this->version_.value <= rhs.getVersion());
    }
};

} // namespace http
} // namespace jimi

#endif // JIMI_HTTP_VERSION_H
