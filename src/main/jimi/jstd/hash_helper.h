
#ifndef JSTD_HASH_HELPER_H
#define JSTD_HASH_HELPER_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "jimi/basic/stddef.h"
#include "jimi/basic/stdint.h"

#include "jimi/Hash.h"
#include "jimi/crc32c.h"

namespace jstd {

enum hash_mode_t {
    Hash_CRC32C,
    Hash_SHA1_MSG2,
    Hash_SHA1,
    Hash_Time31,
    Hash_Time31Std
};

template <std::size_t Mode>
struct hash_helper {
    static uint32_t getHash(const char * data, size_t length) {
        return jimi::crc32c_x64(data, length);
    }
};

template <>
struct hash_helper<Hash_SHA1_MSG2> {
    static uint32_t getHash(const char * data, size_t length) {
        return jimi::sha1_msg2(data, length);
    }
};

template <>
struct hash_helper<Hash_SHA1> {
    static uint32_t getHash(const char * data, size_t length) {
        //alignas(16) uint32_t sha1_state[5];
        //memcpy((void *)&sha1_state[0], (const void *)&jimi::s_sha1_state[0], sizeof(uint32_t) * 5);
        return jimi::sha1_x86(jimi::s_sha1_state, data, length);
    }
};

template <>
struct hash_helper<Hash_Time31> {
    static uint32_t getHash(const char * data, size_t length) {
        return TiStore::hash::Times31(data, length);
    }
};

template <>
struct hash_helper<Hash_Time31Std> {
    static uint32_t getHash(const char * data, size_t length) {
        return TiStore::hash::Times31_std(data, length);
    }
};

} // namespace jstd

#endif // JSTD_HASH_HELPER_H
