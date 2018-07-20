
#ifndef JSTD_DICTIONARY_TRAITS_H
#define JSTD_DICTIONARY_TRAITS_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "jimi/basic/stddef.h"
#include "jimi/basic/stdint.h"

#include "jimi/jstd/hash_helper.h"

namespace jstd {

template <typename Key, typename Value, std::size_t HashFunc = Hash_CRC32C>
struct dictionary_traits {
public:
    typedef Key                         key_type;
    typedef Value                       value_type;
    typedef std::size_t                 size_type;
    typedef std::uint32_t               hash_type;
    typedef std::uint32_t               index_type;
    typedef dictionary_traits<Key, Value, HashFunc>
                                        this_type;

    dictionary_traits() {}
    ~dictionary_traits() {}

    hash_type hash_code(const key_type & key) {
        return jstd::hash_helper::getHashCode()
    }

    index_type index_for(hash_type hash, size_type capacity_mask) {
        return (index_type)((size_type)hash & capacity_mask);
    }

    index_type next_index(index_type index, size_type capacity_mask) {
        ++index;
        return this->index_for((hash_type)index, capacity_mask);
    }

    bool key_is_equal(const key_type & key1, const key_type & key2) const {
        return StrUtils::is_equal(key1.c_str(), key2.c_str(), key1.size());
    }

    bool value_is_equal(const value_type & value1, const value_type & value2) const {
        return StrUtils::is_equal(value1.c_str(), value2.c_str(), value1.size());
    }

    int key_compare(const key_type & key1, const key_type & key2) const {
        return StrUtils::compare(key1.c_str(), key1.size(), key2.c_str(), key2.size());
    }

    int value_compare(const value_type & value1, const value_type & key2) const {
        return StrUtils::compare(value1.c_str(), value1.size(), value2.c_str(), value2.size());
    }

}; // dictionary<K, V>

} // namespace jstd

#endif // JSTD_DICTIONARY_TRAITS_H
