
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
    Hash_Default,
    Hash_CRC32C,
    Hash_Time31,
    Hash_Time31Std,
    Hash_SHA1_MSG2,
    Hash_SHA1,
    Hash_Last
};

template <typename T, typename HashType = std::uint32_t,
          std::size_t HashFunc = Hash_Default>
struct hash_helper {
    typedef typename std::remove_pointer<
                typename std::remove_cv<T>::type
            >::type         Object;

    static
    typename std::enable_if<!std::is_pointer<T>::value, HashType>::type
    getHashCode(const Object & object) {
        return TiStore::hash::Times31_std((const char *)&object, sizeof(object));
    }

    static
    typename std::enable_if<std::is_pointer<T>::value, HashType>::type
    getHashCode(const Object * object) {
        return TiStore::hash::Times31_std((const char *)object, sizeof(Object *));
    }
};

#if SUPPORT_SSE42_CRC32C

template <>
struct hash_helper<const char *, std::uint32_t, Hash_CRC32C> {
    static std::uint32_t getHashCode(const char * data, size_t length) {
        return jimi::crc32c_x64(data, length);
    }
};

template <>
struct hash_helper<std::string, std::uint32_t, Hash_CRC32C> {
    static std::uint32_t getHashCode(const std::string & key) {
        return jimi::crc32c_x64(key.c_str(), key.size());
    }
};

#endif // SUPPORT_SSE42_CRC32C

template <>
struct hash_helper<const char *, std::uint32_t, Hash_Time31> {
    static std::uint32_t getHashCode(const char * data, size_t length) {
        return TiStore::hash::Times31(data, length);
    }
};

template <>
struct hash_helper<std::string, std::uint32_t, Hash_Time31> {
    static std::uint32_t getHashCode(const std::string & key) {
        return TiStore::hash::Times31(key.c_str(), key.size());
    }
};

template <>
struct hash_helper<const char *, std::uint32_t, Hash_Time31Std> {
    static std::uint32_t getHashCode(const char * data, size_t length) {
        return TiStore::hash::Times31_std(data, length);
    }
};

template <>
struct hash_helper<std::string, std::uint32_t, Hash_Time31Std> {
    static std::uint32_t getHashCode(const std::string & key) {
        return TiStore::hash::Times31_std(key.c_str(), key.size());
    }
};

#if SUPPORT_SMID_SHA

template <>
struct hash_helper<const char *, std::uint32_t, Hash_SHA1_MSG2> {
    static std::uint32_t getHashCode(const char * data, size_t length) {
        return jimi::sha1_msg2(data, length);
    }
};

template <>
struct hash_helper<std::string, std::uint32_t, Hash_SHA1_MSG2> {
    static std::uint32_t getHashCode(const std::string & key) {
        return jimi::sha1_msg2(key.c_str(), key.size());
    }
};

template <>
struct hash_helper<const char *, std::uint32_t, Hash_SHA1> {
    static std::uint32_t getHashCode(const char * data, size_t length) {
        //alignas(16) uint32_t sha1_state[5];
        //memcpy((void *)&sha1_state[0], (const void *)&jimi::s_sha1_state[0], sizeof(uint32_t) * 5);
        return jimi::sha1_x86(jimi::s_sha1_state, data, length);
    }
};

template <>
struct hash_helper<std::string, std::uint32_t, Hash_SHA1> {
    static std::uint32_t getHashCode(const std::string & key) {
        //alignas(16) uint32_t sha1_state[5];
        //memcpy((void *)&sha1_state[0], (const void *)&jimi::s_sha1_state[0], sizeof(uint32_t) * 5);
        return jimi::sha1_x86(jimi::s_sha1_state, key.c_str(), key.size());
    }
};

#endif // SUPPORT_SMID_SHA

} // namespace jstd

#endif // JSTD_HASH_HELPER_H
