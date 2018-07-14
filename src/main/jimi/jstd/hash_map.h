
#ifndef JSTD_HASH_MAP_H
#define JSTD_HASH_MAP_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "jimi/basic/stddef.h"
#include "jimi/basic/stddef.h"

#include <cstddef>
#include <memory>
#include <type_traits>

#include "jimi/jstd/hash_helper.h"
#include "jimi/jstd/string_utils.h"

#include "jimi/support/Power2.h"

#define USE_SSE42_STRING_COMPARE    1

namespace jstd {

template <typename Key, typename Value>
struct hash_map_entry {
    typedef Key                         key_type;
    typedef Value                       value_type;
    typedef std::pair<Key, Value>       pair_type;
    typedef std::size_t                 size_type;
    typedef std::uint32_t               hash_type;
    typedef hash_map_entry<Key, Value>  this_type;

    hash_type   hash;
    this_type * next;
    pair_type   pair;

    hash_map_entry() : hash(0), next(nullptr) {}
    hash_map_entry(hash_type init_hash) : hash(init_hash), next(nullptr) {}
    hash_map_entry(hash_type init_hash, const key_type & key,
                   const value_type & value, this_type * next_entry = nullptr)
        : hash(init_hash), next(next_entry), pair(key, value) {}
    hash_map_entry(hash_type init_hash, key_type && key,
                   value_type && value, this_type * next_entry = nullptr)
        : hash(init_hash), next(next_entry),
          pair(std::forward<key_type>(key), std::forward<value_type>(value) {}
    hash_map_entry(const key_type & key, const value_type & value)
        : hash(0), next(nullptr), pair(key, value) {}
    hash_map_entry(key_type && key, value_type && value)
        : hash(0), next(nullptr),
          pair(std::forward<key_type>(key), std::forward<value_type>(value)) {}
    ~hash_map_entry() {}
};

template <typename Key, typename Value, std::size_t Mode = Hash_CRC32C>
class basic_hash_map {
public:
    typedef Key                                 key_type;
    typedef Value                               value_type;
    typedef std::pair<Key, Value>               pair_type;
    typedef std::size_t                         size_type;
    typedef std::uint32_t                       hash_type;

    typedef hash_map_entry<Key, Value>          node_type;
    typedef hash_map_entry<Key, Value> *        data_type;
    typedef data_type *                         iterator;
    typedef const data_type *                   const_iterator;
    typedef basic_hash_map<Key, Value, Mode>    this_type;

private:
    size_type used_;
    size_type size_;
    size_type capacity_;
    data_type * table_;

    static const size_type kInitialCapacity = 64;

public:
    basic_hash_map() :
        used_(0), size_(0), capacity_(0), table_(nullptr) {
        this->init(kInitialCapacity);
    }
    ~basic_hash_map() {
        this->destroy();
    }

    iterator begin() const { return &(this->table_[0]); }
    iterator end() const { return &(this->table_[this->capacity_]); }

    size_type size() const { return this->size_; }
    size_type bucket_used() const { return this->used_; }
    size_type bucket_count() const { return this->capacity_; }
    data_type * data() const { return this->table_; }

    bool empty() const { return (this->size() == 0); }

    void destroy() {
        // Clear all data, and free the table.
        if (likely(this->table_ != nullptr)) {
            for (size_type i = 0; i < this->size_; ++i) {
                node_type * node = (node_type *)this->table_[i];
                if (likely(node != nullptr)) {
                    delete node;
                    this->table_[i] = nullptr;
                }
            }
            delete[] this->table_;
            this->table_ = nullptr;
        }
        this->used_ = 0;
        this->size_ = 0;
        this->capacity_ = 0;
    }

    void clear() {
        // Clear the data only, don't free the table.
        if (likely(this->table_ != nullptr)) {
            for (size_type i = 0; i < this->size_; ++i) {
                node_type * node = (node_type *)this->table_[i];
                if (likely(node != nullptr)) {
                    delete node;
                    this->table_[i] = nullptr;
                }
            }
        }
        this->used_ = 0;
        this->size_ = 0;
    }

private:
    void init(size_type new_capacity) {
        assert(new_capacity > 0);
        assert((new_capacity & (new_capacity - 1)) == 0);
        data_type * new_table = new data_type[new_capacity];
        if (new_table != nullptr) {
            memset(new_table, 0, sizeof(data_type) * new_capacity);
            this->used_ = 0;
            this->size_ = 0;
            this->capacity_ = new_capacity;
            this->table_ = new_table;
        }
    }

    inline size_type calc_capacity(size_type new_capacity) {
        // If new_capacity is less than half of the current hash table size,
        // then double the hash table size.
        new_capacity = (new_capacity > (this->size_ * 2)) ? new_capacity : (this->size_ * 2);
        // The minimum bucket is kBucketsInit = 64.
        new_capacity = (new_capacity >= kInitialCapacity) ? new_capacity : kInitialCapacity;
        // Round up the new_capacity to power 2.
        new_capacity = jimi::detail::round_up_pow2(new_capacity);
        return new_capacity;
    }

    inline size_type calc_capacity_fast(size_type new_capacity) {
        // If new_capacity is less than half of the current hash table size,
        // then double the hash table size.
        new_capacity = (new_capacity > (this->size_ * 2)) ? new_capacity : (this->size_ * 2);
        // Round up the new_capacity to power 2.
        new_capacity = jimi::detail::round_up_pow2(new_capacity);
        return new_capacity;
    }

    void reserve_internal(size_type new_capacity) {
        assert(new_capacity > 0);
        assert((new_capacity & (new_capacity - 1)) == 0);
        if (likely(new_capacity > this->capacity_)) {
            data_type * new_table = new data_type[new_capacity];
            if (new_table != nullptr) {
                memset(new_table, 0, sizeof(data_type) * new_capacity);
                if (likely(this->table_ != nullptr)) {
                    delete[] this->table_;
                }
                this->used_ = 0;
                this->size_ = 0;
                this->capacity_ = new_capacity;
                this->table_ = new_table;
            }
        }
    }

    void inline rehash_insert(data_type * new_table, size_type new_capacity,
                              data_type old_data) {
        assert(new_table != nullptr);
        assert(old_data != nullptr);
        assert(new_capacity > 1);

        const std::string & key = old_data->pair.first;
        hash_type hash = hash_helper<Mode>::getHash(key.c_str(), key.size());
        hash_type bucket = hash % new_capacity;

        // Update the hash value
        old_data->hash = hash;

        if (likely(new_table[bucket] == nullptr)) {
            new_table[bucket] = old_data;
        }
        else {
            do {
                bucket = (bucket + 1) % new_capacity;
                if (likely(new_table[bucket] == nullptr)) {
                    new_table[bucket] = old_data;
                    break;
                }
            } while (1);
        }
    }

    void rehash_internal(size_type new_capacity) {
        assert(new_capacity > 0);
        assert((new_capacity & (new_capacity - 1)) == 0);
        assert(new_capacity >= this->size_ * 2);
        if (likely(new_capacity > this->capacity_)) {
            data_type * new_table = new data_type[new_capacity];
            if (likely(new_table != nullptr)) {
                // Initialize new table.
                memset(new_table, 0, sizeof(data_type) * new_capacity);

                if (likely(this->table_ != nullptr)) {
                    // Recalculate all hash values.
                    size_type new_size = 0;

                    for (size_type i = 0; i < this->capacity_; ++i) {
                        if (likely(this->table_[i] != nullptr)) {
                            // Insert the old buckets to the new buckets in the new table.
                            this->rehash_insert(new_table, new_capacity, this->table_[i]);
                            ++new_size;
                        }
                    }
                    assert(new_size == this->size_);

                    // Free old table data.
                    delete[] this->table_;
                }
                this->table_ = new_table;
                this->used_ = 0;
                this->capacity_ = new_capacity;
            }
        }
    }

    void shrink_internal(size_type new_capacity) {
        assert(new_capacity > 0);
        assert((new_capacity & (new_capacity - 1)) == 0);
        assert(new_capacity >= this->size_ * 2);
        if (likely(new_capacity != this->capacity_)) {
            data_type * new_table = new data_type[new_capacity];
            if (likely(new_table != nullptr)) {
                // Initialize new table.
                memset(new_table, 0, sizeof(data_type) * new_capacity);

                if (likely(this->table_ != nullptr)) {
                    // Recalculate all hash values.
                    size_type new_size = 0;

                    for (size_type i = 0; i < this->capacity_; ++i) {
                        if (likely(this->table_[i] != nullptr)) {
                            // Insert the old buckets to the new buckets in the new table.
                            this->rehash_insert(new_table, new_capacity, this->table_[i]);
                            ++new_size;
                        }
                    }
                    assert(new_size == this->size_);

                    // Free old table data.
                    delete[] this->table_;
                }
                this->table_ = new_table;
                this->used_ = 0;
                this->capacity_ = new_capacity;
            }
        }
    }

    void resize_internal(size_type new_capacity) {
        assert(new_capacity > 0);
        assert((new_capacity & (new_capacity - 1)) == 0);
        rehash_internal(new_capacity);
    }

public:
    void reserve(size_type new_capacity) {
        // Recalculate the size of new_capacity.
        new_capacity = this->calc_capacity(new_capacity);
        this->reserve_internal(new_capacity);
    }

    void resize(size_type new_capacity) {
        // Recalculate the size of new_capacity.
        new_capacity = this->calc_capacity(new_capacity);
        this->resize_internal(new_capacity);
    }

    void rehash(size_type new_capacity) {
        // Recalculate the size of new_capacity.
        new_capacity = this->calc_capacity(new_capacity);
        this->rehash_internal(new_capacity);
    }

    void shrink_to(size_type new_capacity) {
        // Recalculate the size of new_capacity.
        new_capacity = this->calc_capacity_fast(new_capacity);
        this->shrink_internal(new_capacity);
    }

    iterator find(const key_type & key) {
        hash_type hash = hash_helper<Mode>::getHash(key.c_str(), key.size());
        hash_type bucket = hash % this->capacity_;
        node_type * node = (node_type *)this->table_[bucket];

        if (likely(node != nullptr)) {
            // Found, next to check the hash value.
            if (likely(node->hash == hash)) {
                // If hash value is equal, then compare the key sizes and the strings.
                if (likely(node->pair.first.size() == key.size())) {
#if USE_SSE42_STRING_COMPARE
                    if (likely(detail::string_equal(node->pair.first.c_str(), key.c_str(), key.size()))) {
                        return (iterator)&this->table_[bucket];
                    }
#else
                    if (likely(strcmp(node->pair.first.c_str(), key.c_str()) == 0)) {
                        return (iterator)&this->table_[bucket];
                    }
#endif
                }
            }
        }

        // If first position is not found, search next bucket continue.
        hash_type first_bucket = bucket;
        do {
            bucket = (bucket + 1) % this->capacity_;
            node = (node_type *)this->table_[bucket];
            if (likely(node != nullptr)) {
                if (likely(node->hash == hash)) {
                    // If hash value is equal, then compare the key sizes and the strings.
                    if (likely(node->pair.first.size() == key.size())) {
#if USE_SSE42_STRING_COMPARE
                        if (likely(detail::string_equal(node->pair.first.c_str(), key.c_str(), key.size()))) {
                            return (iterator)&this->table_[bucket];
                        }
#else
                        if (likely(strcmp(node->pair.first.c_str(), key.c_str()) == 0)) {
                            return (iterator)&this->table_[bucket];
                        }
#endif
                    }
                }
            }
        } while (likely(bucket != first_bucket));

        // Not found
        return this->end();
    }

    void insert(const key_type & key, const value_type & value) {
        iterator iter = this->find(key);
        if (likely(iter == this->end())) {
            // Insert the new key.
            if (unlikely(this->size_ >= (this->capacity_ * 3 / 4))) {
                this->resize_internal(this->capacity_ * 2);
            }

            hash_type hash = hash_helper<Mode>::getHash(key.c_str(), key.size());
            node_type * new_data = new node_type(hash, key, value);
            if (likely(new_data != nullptr)) {
                hash_type bucket = hash % this->capacity_;
                if (likely(this->table_[bucket] == nullptr)) {
                    this->table_[bucket] = (data_type)new_data;
                    ++(this->size_);
                }
                else {
                    do {
                        bucket = (bucket + 1) % this->capacity_;
                        if (likely(this->table_[bucket] == nullptr)) {
                            this->table_[bucket] = (data_type)new_data;
                            ++(this->size_);
                            break;
                        }
                    } while (1);
                }
            }
        }
        else {
            // Update the existed key's value.
            (*iter)->pair.second = value;
        }
    }

    void insert(key_type && key, value_type && value) {
        iterator iter = this->find(key);
        if (likely(iter == this->end())) {
            // Insert the new key.
            if (unlikely(this->size_ >= (this->capacity_ * 3 / 4))) {
                this->resize_internal(this->capacity_ * 2);
            }

            hash_type hash = hash_helper<Mode>::getHash(key.c_str(), key.size());
            node_type * new_data = new node_type(hash, forward<key_type>(key),
                                                 std::forward<value_type>(value));
            if (likely(new_data != nullptr)) {
                hash_type bucket = hash % this->capacity_;
                if (likely(this->table_[bucket] == nullptr)) {
                    this->table_[bucket] = (data_type)new_data;
                    ++(this->size_);
                }
                else {
                    do {
                        bucket = (bucket + 1) % this->capacity_;
                        if (likely(this->table_[bucket] == nullptr)) {
                            this->table_[bucket] = (data_type)new_data;
                            ++(this->size_);
                            break;
                        }
                    } while (1);
                }
            }
        }
        else {
            // Update the existed key's value.
            (*iter)->pair.second = std::move(std::forward<value_type>(value));
        }
    }

    void insert(const pair_type & pair) {
        this->insert(pair.first, pair.second);
    }

    void emplace(const pair_type & pair) {
        this->insert(pair.first, pair.second);
    }

    void erase(const key_type & key) {
        iterator iter = this->find(key);
        if (likely(iter != this->end())) {
            assert(this->size_ > 0);
            if (likely(iter != nullptr)) {
                if (likely(*iter != nullptr)) {
                    delete *iter;
                    *iter = nullptr;
                    assert(this->size_ > 0);
                    --(this->size_);
                }
            }
        }
        else {
            // Not found the key
        }
    }

    void erase(key_type && key) {
        iterator iter = this->find(std::forward<key_type>(key));
        if (likely(iter != this->end())) {
            assert(this->size_ > 0);
            if (likely(iter != nullptr)) {
                if (likely(*iter != nullptr)) {
                    delete *iter;
                    *iter = nullptr;
                    assert(this->size_ > 0);
                    --(this->size_);
                }
            }
        }
        else {
            // Not found the key
        }
    }

    static const char * name() {
        switch (Mode) {
        case Hash_CRC32C:
            return "jstd::hash_map<std::string, std::string>";
        case Hash_SHA1_MSG2:
            return "jstd::hash_map_v1<std::string, std::string>";
        case Hash_SHA1:
            return "jstd::hash_map_v2<std::string, std::string>";
        case Hash_Time31:
            return "jstd::hash_map_v3<std::string, std::string>";
        case Hash_Time31Std:
            return "jstd::hash_map_v4<std::string, std::string>";
        default:
            return "Unknown class name";
        }
    }
};

template <typename Key, typename Value>
using hash_map = basic_hash_map<Key, Value, Hash_CRC32C>;

#if USE_SHA1_HASH
template <typename Key, typename Value>
using hash_map_v1 = basic_hash_map<Key, Value, Hash_SHA1_MSG2>;
#endif

#if USE_SHA1_HASH
template <typename Key, typename Value>
using hash_map_v2 = basic_hash_map<Key, Value, Hash_SHA1>;
#endif

template <typename Key, typename Value>
using hash_map_v3 = basic_hash_map<Key, Value, Hash_Time31>;

template <typename Key, typename Value>
using hash_map_v4 = basic_hash_map<Key, Value, Hash_Time31Std>;

} // namespace jstd

#endif // JSTD_HASH_MAP_H
