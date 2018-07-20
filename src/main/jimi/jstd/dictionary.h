
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

    typedef entry               entry_type;
    typedef entry_type *        iterator;
    typedef const entry_type *  const_iterator;

private:
    entry_type ** buckets_;
    entry_type * entries_;
    size_type mask_;
    size_type capacity_;
    size_type size_;
    size_type threshold_;
    float loadFactor_;
    traits_type traits_;

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
        : buckets_(nullptr), entries_(nullptr), mask_(0), capacity_(0), size_(0),
          threshold_(0), loadFactor_(kDefaultLoadFactor) {
        this->initialize(initialCapacity, loadFactor);
    }
    ~basic_dictionary() {
        this->destroy();
    }

    iterator begin() const {
        return (this->entries() != nullptr) ? (iterator)&this->entries_[0] : nullptr;
    }
    iterator end() const {
        return (this->entries() != nullptr) ? (iterator)&this->entries_[this->entries_count()] : nullptr;
    }

    iterator unsafe_begin() const {
        return (iterator)&this->entries_[0];
    }
    iterator unsafe_end() const {
        return (iterator)&this->entries_[this->entries_count()];
    }

    size_type size() const { return this->size_; }
    size_type bucket_mask() const { return this->mask_; }
    size_type bucket_capacity() const { return this->capacity_; }
    size_type entries_count() const { return this->capacity_; }
    entry_type ** buckets() const { return this->buckets_; }
    entry_type * entries() const { return this->entries_; }

    size_type threshold() const { return this->threshold_; }
    float load_factor() const { return this->loadFactor_; }

    bool is_valid() const { return (this->buckets_ != nullptr); }
    bool is_empty() const { return (this->size() == 0); }

    void clear() {
        if (likely(this->buckets_ != nullptr)) {
            // Initialize the buckets's data.
            memset((void *)this->buckets_, 0, sizeof(entry_type *) * this->capacity_);
        }
        // Setting status
        this->size_ = 0;
    }

private:
    void initialize(size_type new_capacity, float loadFactor) {
        new_capacity = jimi::detail::round_up_pow2(new_capacity);
        assert(new_capacity > 0);
        assert((new_capacity & (new_capacity - 1)) == 0);
        // The the array of bucket's first entry.
        entry_type ** new_buckets = new entry_type *[new_capacity];
        if (likely(new_buckets != nullptr)) {
            // Initialize the buckets's data.
            memset((void *)new_buckets, 0, sizeof(entry_type *) * new_capacity);
            // Record this->buckets_
            this->buckets_ = new_buckets;
            // The the array of entries.
            entry_type * new_entries = new entry_type[new_capacity];
            if (likely(new_entries != nullptr)) {
                // Initialize status
                this->entries_ = new_entries;
                this->mask_ = new_capacity - 1;
                this->capacity_ = new_capacity;
                this->size_ = 0;
                assert(loadFactor > 0.0f);
                this->threshold_ = (size_type)(new_capacity * fabsf(loadFactor));
                this->loadFactor_ = loadFactor;
            }
        }
    }

    void destroy() {
        // Free the resources.
        if (likely(this->buckets_ != nullptr)) {
            if (likely(this->entries_ != nullptr)) {
                // Free all entries.
                delete[] this->entries_;
                this->entries_ = nullptr;
            }
            // Free the array of bucket's first entry.
            delete[] this->buckets_;
            this->buckets_ = nullptr;
        }
#ifdef NDEBUG
        // Setting status
        this->mask_ = 0;
        this->capacity_ = 0;
        this->size_ = 0;
        this->threshold_ = 0;
#endif
    }

public:
    iterator find(const key_type & key) {
        if (likely(this->buckets() != nullptr)) {
            hash_type hash = traits_.hash_code(key);
            size_type index = traits_.index_for(hash, this->capacity_);

            assert(this->entries() != nullptr);
            entry_type * entry = this->buckets_[index];
            while (likely(entry != nullptr)) {
                // Found entry, next to check the hash value.
                if (likely(entry->hash == hash)) {
                    // If hash value is equal, then compare the key sizes and the strings.
                    if (likely(traits_.key_is_equal(key, entry->pair.first))) {
                        return (iterator)entry;
                    }
                }
                // Scan next entry
                entry = entry->next;
            }

            // Not found
            return this->unsafe_end();
        }

        // Not found
        return nullptr;
    }

    bool contains(const key_type & key) {
        iterator iter = this->find(key);
        return (iter != this->end());
    }

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
