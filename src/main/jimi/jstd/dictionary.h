
#ifndef JSTD_DICTIONARY_H
#define JSTD_DICTIONARY_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "jimi/jstd/hash_helper.h"
#include "jimi/jstd/dictionary_traits.h"

namespace jstd {

template <typename Key, typename Value, std::size_t HashFunc = Hash_CRC32C,
          typename Traits = dictionary_traits<Key, Value, HashFunc>>
class basic_dictionary {
public:
    typedef Key                             key_type;
    typedef Value                           value_type;
    typedef Traits                          traits_type;
    typedef std::pair<Key, Value>           pair_type;
    typedef typename Traits::size_type      size_type;
    typedef typename Traits::hash_type      hash_type;
    typedef typename Traits::index_type     index_type;
    typedef basic_dictionary<Key, Value, HashFunc, Traits>
                                            this_type;

    struct entry {
        entry *     next;
        hash_type   hash;
        pair_type   pair;

        entry() : next(nullptr), hash(0) {}
        entry(hash_type hash_code) : next(nullptr), hash(hash_code) {}

        entry(hash_type hash_code, const key_type & key,
              const value_type & value, this_type * next_entry = nullptr)
            : next(next_entry), hash(hash_code), pair(key, value) {}
        entry(hash_type hash_code, key_type && key,
              value_type && value, this_type * next_entry = nullptr)
            : next(next_entry), hash(hash_code),
              pair(std::forward<key_type>(key), std::forward<value_type>(value)) {}

        entry(const key_type & key, const value_type & value)
            : next(nullptr), hash(0), pair(key, value) {}
        entry(key_type && key, value_type && value)
            : next(nullptr), hash(0),
              pair(std::forward<key_type>(key), std::forward<value_type>(value)) {}

        ~entry() {
#ifndef NDEBUG
            this->next = nullptr;
#endif
        }
    };

    typedef entry                           entry_type;
    typedef entry_type *                    iterator;
    typedef const entry_type *              const_iterator;

private:
    index_type * buckets_;
    size_type mask_;
    size_type capacity_;
    size_type size_;
    size_type threshold_;
    float loadFactor_;

    // Default initial capacity is 64.
    static const size_type kDefaultInitialCapacity = 64;
    // Maximum capacity is 1 << 30.
    static const size_type kMaximumCapacity = 1U << 30;
    // Default load factor is: 0.75
    static const float kDefaultLoadFactor;
    // The threshold of treeify to red-black tree.
    static const size_type kTreeifyThreshold = 8;

public:
    basic_dictionary(size_type initialCapacity = kDefaultInitialCapacity,
                     float loadFactor = kDefaultLoadFactor)
        : buckets_(nullptr), mask_(0), capacity_(0), size_(0),
          threshold_(0), loadFactor_(kDefaultLoadFactor) {
        this->initialize(initialCapacity);
    }
    ~basic_dictionary() {}

private:
    void initialize(size_type new_capacity) {
        //
    }

public:
    void dump() {
        printf("jstd::basic_dictionary<K, V>::dump()\n\n");
    }

}; // dictionary<K, V>

template <typename Key, typename Value, std::size_t HashFunc, typename Traits>
const float basic_dictionary<Key, Value, HashFunc, Traits>::kDefaultLoadFactor = 0.75f;

template <typename Key, typename Value>
using dictionary = basic_dictionary<Key, Value, Hash_CRC32C>;

template <typename Key, typename Value>
using dictionary_v1 = basic_dictionary<Key, Value, Hash_Time31>;

template <typename Key, typename Value>
using dictionary_v2 = basic_dictionary<Key, Value, Hash_Time31Std>;

#if USE_SHA1_HASH
template <typename Key, typename Value>
using dictionary_v3 = basic_dictionary<Key, Value, Hash_SHA1_MSG2>;
#endif

#if USE_SHA1_HASH
template <typename Key, typename Value>
using dictionary_v4 = basic_dictionary<Key, Value, Hash_SHA1>;
#endif

} // namespace jstd

#endif // JSTD_DICTIONARY_H
