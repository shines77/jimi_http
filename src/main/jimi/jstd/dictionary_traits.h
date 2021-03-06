
#ifndef JSTD_DICTIONARY_TRAITS_H
#define JSTD_DICTIONARY_TRAITS_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "jimi/basic/stddef.h"
#include "jimi/basic/stdint.h"
#include "jimi/basic/stdsize.h"

#include <string.h>
#include <string>

#include "jimi/jstd/hash_helper.h"
#include "jimi/jstd/string_utils.h"

namespace jstd {

//
// Default jstd::dictionary<K, V> hash traits
//
template <typename Key, typename Value, std::size_t HashFunc = HashFunc_Default>
struct default_dictionary_hasher {
    typedef Key             key_type;
    typedef Value           value_type;
    typedef std::size_t     size_type;
    typedef std::uint32_t   hash_type;
    typedef std::size_t     index_type;

    // The invalid hash value.
    static const hash_type kInvalidHash = static_cast<hash_type>(-1);
    // The replacement value for invalid hash value.
    static const hash_type kReplacedHash = static_cast<hash_type>(-2);

    default_dictionary_hasher() {}
    ~default_dictionary_hasher() {}

    hash_type hash_code(const key_type & key) const {
        // All of the following two methods can get the hash value.
#if 0
        hash_type hash = jstd::hash_helper<key_type, hash_type, HashFunc>::getHashCode(key);
#else
        jstd::hash<key_type, hash_type, HashFunc> hasher;
        hash_type hash = hasher(key);
#endif
        // The hash code can't equal to kInvalidHash, replacement to kReplacedHash.
        if (likely(hash != kInvalidHash))
            return hash;
        else
            return kReplacedHash;
    }

    index_type index_of(hash_type hash, size_type capacity_mask) const {
        return (index_type)((size_type)hash & capacity_mask);
    }

    index_type next_index(index_type index, size_type capacity_mask) const {
        ++index;
        return (index_type)((size_type)index & capacity_mask);
    }
}; // struct default_dictionary_hasher<K, V, HashFunc>

//
// Default jstd::dictionary<K, V> compare traits
//
template <typename Key, typename Value>
struct default_dictionary_comparer {
    typedef Key     key_type;
    typedef Value   value_type;

    default_dictionary_comparer() {}
    ~default_dictionary_comparer() {}

#if (STRING_COMPARE_MODE == STRING_COMPARE_STDC)

    bool key_is_equals(const key_type & key1, const key_type & key2) const {
        return (strcmp(key1.c_str(), key2. c_str()) == 0);
    }

    bool value_is_equals(const value_type & value1, const value_type & value2) const {
        return (strcmp(value1.c_str(), value2. c_str()) == 0);
    }

    int key_compare(const key_type & key1, const key_type & key2) const {
        return strcmp(key1.c_str(), key2. c_str());
    }

    int value_compare(const value_type & value1, const value_type & value2) const {
        return strcmp(value1.c_str(), value2. c_str());
    }

#else // (STRING_COMPARE_MODE != STRING_COMPARE_STDC)

    bool key_is_equals(const key_type & key1, const key_type & key2) const {
        return StrUtils::is_equals(key1, key2);
    }

    bool value_is_equals(const value_type & value1, const value_type & value2) const {
        return StrUtils::is_equals(value1, value2);
    }

    int key_compare(const key_type & key1, const key_type & key2) const {
        return StrUtils::compare(key1, key2);
    }

    int value_compare(const value_type & value1, const value_type & value2) const {
        return StrUtils::compare(value1, value2);
    }

#endif // STRING_COMPARE_MODE
}; // struct default_dictionary_comparer<K, V>

template <typename T>
inline static
int comparer(const T & v1, const T & v2) {
    return ((v1 > v2) ? 1 : ((v1 < v2) ? -1 : 0));
}

template <>
struct default_dictionary_comparer<int, int> {
    typedef int key_type;
    typedef int value_type;

    default_dictionary_comparer() {}
    ~default_dictionary_comparer() {}

    bool key_is_equals(const key_type & key1, const key_type & key2) const {
        return (key1 == key2);
    }

    bool value_is_equals(const value_type & value1, const value_type & value2) const {
        return (value1 == value2);
    }

    int key_compare(const key_type & key1, const key_type & key2) const {
        return comparer(key1, key2);
    }

    int value_compare(const value_type & value1, const value_type & value2) const {
        return comparer(value1, value2);
    }
};

template <typename Value>
struct default_dictionary_comparer<int, Value> {
    typedef int     key_type;
    typedef Value   value_type;

    default_dictionary_comparer() {}
    ~default_dictionary_comparer() {}

    bool key_is_equals(const key_type & key1, const key_type & key2) const {
        return (key1 == key2);
    }

    bool value_is_equals(const value_type & value1, const value_type & value2) const {
        return (value1 == value2);
    }

    int key_compare(const key_type & key1, const key_type & key2) const {
        return comparer(key1, key2);
    }

    int value_compare(const value_type & value1, const value_type & value2) const {
        return comparer(value1, value2);
    }
};

//
// Default jstd::dictionary<K, V> traits
//
template <typename Key, typename Value, std::size_t HashFunc = HashFunc_Default,
          typename Hasher = default_dictionary_hasher<Key, Value, HashFunc>,
          typename Comparer = default_dictionary_comparer<Key, Value>>
struct default_dictionary_traits {
    typedef Key                             key_type;
    typedef Value                           value_type;
    typedef typename Hasher::size_type      size_type;
    typedef typename Hasher::hash_type      hash_type;
    typedef typename Hasher::index_type     index_type;

    Hasher hasher_;
    Comparer comparer_;

    default_dictionary_traits() {}
    ~default_dictionary_traits() {}

    //
    // Hasher
    //
    hash_type hash_code(const key_type & key) const {
        return this->hasher_.hash_code(key);
    }

    index_type index_of(hash_type hash, size_type capacity_mask) const {
        return this->hasher_.index_of(hash, capacity_mask);
    }

    index_type next_index(index_type index, size_type capacity_mask) const {
        return this->hasher_.next_index(index, capacity_mask);
    }

    //
    // Comparer
    //
    bool key_is_equals(const key_type & key1, const key_type & key2) const {
        return this->comparer_.key_is_equals(key1, key2);
    }

    bool value_is_equals(const value_type & value1, const value_type & value2) const {
        return this->comparer_.value_is_equals(value1, value2);
    }

    int key_compare(const key_type & key1, const key_type & key2) const {
        return this->comparer_.key_compare(key1, key2);
    }

    int value_compare(const value_type & value1, const value_type & value2) const {
        return this->comparer_.value_compare(value1, value2);
    }

}; // struct default_dictionary_traits<K, V, HashFunc, Hasher, Comparer>

} // namespace jstd

#endif // JSTD_DICTIONARY_TRAITS_H
