
#ifndef JSTD_HASH_TABLE_H
#define JSTD_HASH_TABLE_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "jimi/basic/stddef.h"

#include <stdint.h>
#include <cstddef>
#include <memory>
#include <type_traits>

#include "jimi/support/bitscan_reverse.h"
#include "jimi/support/bitscan_forward.h"

namespace jstd {

namespace detail {

std::size_t round_to_pow2(std::size_t n)
{
    unsigned long index;
#if defined(WIN64) || defined(_WIN64) || defined(_M_X64) || defined(_M_AMD64) \
 || defined(_M_IA64) || defined(__amd64__) || defined(__x86_64__) \
 || defined(_M_ARM64)
    unsigned char nonZero = __BitScanReverse64(index, n);
    return (nonZero ? (1ULL << index) : 1ULL);
#else
    unsigned char nonZero = __BitScanReverse(index, n);
    return (nonZero ? (1UL << index) : 1UL);
#endif
}

std::size_t round_up_pow2(std::size_t n)
{
    unsigned long index;
#if defined(WIN64) || defined(_WIN64) || defined(_M_X64) || defined(_M_AMD64) \
 || defined(_M_IA64) || defined(__amd64__) || defined(__x86_64__) \
 || defined(_M_ARM64)
    unsigned char nonZero = __BitScanReverse64(index, n);
    return (nonZero ? (1ULL << (index + 1)) : 2ULL);
#else
    unsigned char nonZero = __BitScanReverse(index, n);
    return (nonZero ? (1UL << (index + 1)) : 2UL);
#endif
}

} // namespace detail

template <typename Key, typename Value>
struct hash_table_node {
    typedef Key                         key_type;
    typedef Value                       value_type;
    typedef std::pair<Key, Value>       pair_type;
    typedef std::size_t                 size_type;
    typedef std::uint32_t               hash_type;
    typedef hash_table_node<Key, Value> this_type;

    hash_type hash;
    pair_type pair;

    hash_table_node() : hash(0) {}
    hash_table_node(const key_type & key, const value_type & value, hash_type init_hash = 0)
        : hash(init_hash), pair(key, value) {}
    hash_table_node(key_type && key, value_type && value, hash_type init_hash = 0)
        : hash(init_hash), pair(std::make_pair(key, value)) {}
    ~hash_table_node() {}
};

template <typename Key, typename Value>
class hash_table {
public:
    typedef Key                             key_type;
    typedef Value                           value_type;
    typedef std::pair<Key, Value>           pair_type;
    typedef std::size_t                     size_type;
    typedef std::uint32_t                   hash_type;

    typedef hash_table_node<Key, Value>     node_type;
    typedef hash_table_node<Key, Value> *   data_type;
    typedef data_type *                     iterator;
    typedef const data_type *               const_iterator;
    typedef hash_table<Key, Value>          this_type;

private:
    data_type * table_;
    size_type size_;
    size_type mask_;
    size_type buckets_;

    static const size_type kBucketsInit = 64;

public:
    hash_table() : table_(nullptr), size_(0), mask_(0), buckets_(0) {
        this->init(kBucketsInit);
    }
    ~hash_table() {
        this->destroy();
    }

    iterator begin() const { return &(this->table_[0]); }
    iterator end() const { return &(this->table_[this->buckets_]); }

private:
    void init(size_type new_buckets) {
        assert(new_buckets > 0);
        assert((new_buckets & (new_buckets - 1)) == 0);
        data_type * new_table = new data_type[new_buckets];
        if (new_table != nullptr) {
            memset(new_table, 0, sizeof(data_type) * new_buckets);
            this->table_ = new_table;
            this->mask_ = new_buckets - 1;
            this->buckets_ = new_buckets;
        }
    }

public:
    void destroy() {
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
        this->size_ = 0;
        this->mask_ = 0;
        this->buckets_ = 0;
    }

private:
    void rehash_insert(data_type * new_table, size_type new_buckets, data_type new_data) {
        assert(new_table != nullptr);
        assert(new_data != nullptr);
        assert(new_buckets > 1);
        size_type new_mask = new_buckets - 1;

        const std::string & key = new_data->pair.first;
        hash_type hash = jimi::crc32_x64(key.c_str(), key.size());
        hash_type index = hash & new_mask;

        // Update the hash value
        new_data->hash = hash;

        if (likely(new_table[index] == nullptr)) {
            new_table[index] = new_data;
        }
        else {
            do {
                index = (index + 1) & new_mask;
                if (likely(new_table[index] == nullptr)) {
                    new_table[index] = new_data;
                    break;
                }
            } while (1);
        }
    }

    void rehash(data_type * new_table, size_type new_buckets) {
        assert(new_table != nullptr);
        assert(new_buckets > this->buckets_);
        size_type new_size = 0;

        for (size_type i = 0; i < this->buckets_; ++i) {
            if (this->table_[i] != nullptr) {
                this->rehash_insert(new_table, new_buckets, this->table_[i]);
                ++new_size;
            }
        }
        assert(new_size == this->size_);
    }

public:
    void reserve_fast(size_type new_buckets) {
        assert(new_buckets > 0);
        assert((new_buckets & (new_buckets - 1)) == 0);
        if (likely(new_buckets > this->buckets_)) {
            data_type * new_table = new data_type[new_buckets];
            if (new_table != nullptr) {
                memset(new_table, 0, sizeof(data_type) * new_buckets);
                if (likely(this->table_ != nullptr)) {
                    delete[] this->table_;
                }
                this->table_ = new_table;
                this->mask_ = new_buckets - 1;
                this->buckets_ = new_buckets;
            }
        }
    }

    void reserve(size_type new_buckets) {
        new_buckets = (new_buckets >= kBucketsInit) ? (new_buckets - 1) : (kBucketsInit - 1);
        size_type new_capacity = detail::round_up_pow2(new_buckets);
        this->reserve_fast(new_capacity);
    }

    void resize_fast(size_type new_buckets) {
        assert(new_buckets > 0);
        assert((new_buckets & (new_buckets - 1)) == 0);
        if (likely(new_buckets > this->buckets_)) {
            data_type * new_table = new data_type[new_buckets];
            if (new_table != nullptr) {
                memset(new_table, 0, sizeof(data_type) * new_buckets);
                if (likely(this->table_ != nullptr)) {
                    // Recalculate all hash value.
                    this->rehash(new_table, new_buckets);

                    // Free old data.
                    delete[] this->table_;
                }
                this->table_ = new_table;
                this->mask_ = new_buckets - 1;
                this->buckets_ = new_buckets;
            }
        }
    }

    void resize(size_type new_buckets) {
        new_buckets = (new_buckets >= kBucketsInit) ? (new_buckets - 1) : (kBucketsInit - 1);
        size_type new_capacity = detail::round_up_pow2(new_buckets);
        this->resize_fast(new_capacity);
    }

    iterator find(const key_type & key) {
        hash_type hash = jimi::crc32_x64(key.c_str(), key.size());
        hash_type index = hash & this->mask_;
        node_type * node = (node_type *)this->table_[index];
        if (likely(node != nullptr)) {
            // Found, next to check the hash value.
            if (likely(node->hash == hash)) {
                // If hash value is equal, compare the key sizes and strings.
                if (likely(node->pair.first.size() == key.size())) {
                    if (likely(strcmp(node->pair.first.c_str(), key.c_str()) == 0)) {
                        return (iterator)&this->table_[index];
                    }
                }
            }

            hash_type first_index = index;
            do {
                index = (index + 1) & this->mask_;
                node = (node_type *)this->table_[index];
                if (likely(node != nullptr && node->hash == hash)) {
                    // If hash value is equal, compare the key sizes and strings.
                    if (likely(node->pair.first.size() == key.size())) {
                        if (likely(strcmp(node->pair.first.c_str(), key.c_str()) == 0)) {
                            return (iterator)&this->table_[index];
                        }
                    }
                }
            } while (likely(index != first_index));
        }

        // Not found
        return this->end();
    }

    void insert(const key_type & key, const value_type & value) {
        iterator iter = this->find(key);
        if (likely(iter == this->end())) {
            // Insert the new key.
            if (unlikely(this->size_ >= (this->buckets_ * 3 / 4))) {
                this->resize_fast(this->buckets_ * 2);
            }

            node_type * new_data = new node_type(key, value);
            if (likely(new_data != nullptr)) {
                hash_type hash = jimi::crc32_x64(key.c_str(), key.size());
                hash_type index = hash & this->mask_;
                new_data->hash = hash;
                if (likely(this->table_[index] == nullptr)) {
                    this->table_[index] = (data_type)new_data;
                    ++(this->size_);
                }
                else {
                    do {
                        index = (index + 1) & this->mask_;
                        if (likely(this->table_[index] == nullptr)) {
                            this->table_[index] = (data_type)new_data;
                            ++(this->size_);
                            break;
                        }
                    } while (1);
                }
            }
        }
        else {
            // Update the existed key.
            (*iter)->pair.second = value;
        }
    }

    void insert(key_type && key, value_type && value) {
        iterator iter = this->find(key);
        if (likely(iter == this->end())) {
            // Insert the new key.
            if (unlikely(this->size_ >= (this->buckets_ * 3 / 4))) {
                this->resize_fast(this->buckets_ * 2);
            }

            node_type * new_data = new node_type(std::forward<key_type>(key),
                                                 std::forward<value_type>(value));
            if (likely(new_data != nullptr)) {
                hash_type hash = jimi::crc32_x64(key.c_str(), key.size());
                hash_type index = hash & this->mask_;
                new_data->hash = hash;
                if (likely(this->table_[index] == nullptr)) {
                    this->table_[index] = (data_type)new_data;
                    ++(this->size_);
                }
                else {
                    do {
                        index = (index + 1) & this->mask_;
                        if (likely(this->table_[index] == nullptr)) {
                            this->table_[index] = (data_type)new_data;
                            ++(this->size_);
                            break;
                        }
                    } while (1);
                }
            }
        }
        else {
            // Update the existed key.
            (*iter)->pair.second = std::move(std::forward<value_type>(value));
        }
    }

    void insert(const pair_type & pair) {
        this->insert(pair.first, pair.second);
    }
};

} // namespace jstd

#endif // JSTD_HASH_TABLE_H
