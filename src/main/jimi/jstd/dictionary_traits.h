
#ifndef JSTD_DICTIONARY_TRAITS_H
#define JSTD_DICTIONARY_TRAITS_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "jimi/basic/stddef.h"
#include "jimi/basic/stdint.h"
#include "jimi/basic/stdsize.h"

#include <string.h>

#include "jimi/jstd/hash_helper.h"
#include "jimi/jstd/string_utils.h"

namespace jstd {

//
// Default jstd::dictionary<K, V> hash traits
//
template <typename Key, typename Value, std::size_t HashFunc = Hash_Default>
struct default_dictionary_hasher {
    typedef Key             key_type;
    typedef Value           value_type;
    typedef std::size_t     size_type;
    typedef std::uint32_t   hash_type;
    typedef std::size_t     index_type;

    default_dictionary_hasher() {}
    ~default_dictionary_hasher() {}

    hash_type hash_code(const key_type & key) const {
        return jstd::hash_helper<HashFunc>::getHashCode(key.c_str(), key.size());
    }

    index_type index_for(hash_type hash, size_type capacity_mask) const {
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

//
// Default jstd::dictionary<K, V> traits
//
template <typename Key, typename Value, std::size_t HashFunc = Hash_Default,
          typename HashTraits = default_dictionary_hasher<Key, Value, HashFunc>,
          typename ComparerTraits = default_dictionary_comparer<Key, Value>>
struct default_dictionary_traits {
    typedef Key                             key_type;
    typedef Value                           value_type;
    typedef typename HashTraits::size_type  size_type;
    typedef typename HashTraits::hash_type  hash_type;
    typedef typename HashTraits::index_type index_type;

    HashTraits hashTraits_;
    ComparerTraits comparerTraits_;

    default_dictionary_traits() {}
    ~default_dictionary_traits() {}

    //
    // HashTraits
    //
    hash_type hash_code(const key_type & key) const {
        return this->hashTraits_.hash_code(key);
    }

    index_type index_for(hash_type hash, size_type capacity_mask) const {
        return this->hashTraits_.index_for(hash, capacity_mask);
    }

    index_type next_index(index_type index, size_type capacity_mask) const {
        return this->hashTraits_.next_index(index, capacity_mask);
    }

    //
    // ComparerTraits
    //
    bool key_is_equals(const key_type & key1, const key_type & key2) const {
        return this->comparerTraits_.key_is_equals(key1, key2);
    }

    bool value_is_equals(const value_type & value1, const value_type & value2) const {
        return this->comparerTraits_.value_is_equals(value1, value2);
    }

    int key_compare(const key_type & key1, const key_type & key2) const {
        return this->comparerTraits_.key_compare(key1, key2);
    }

    int value_compare(const value_type & value1, const value_type & value2) const {
        return this->comparerTraits_.value_compare(value1, value2);
    }

}; // struct default_dictionary_traits<K, V, HashFunc, HashTraits, ComparerTraits>

} // namespace jstd

#endif // JSTD_DICTIONARY_TRAITS_H
