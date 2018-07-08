
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

#include <nmmintrin.h>  // For SSE 4.2
#include "jimi/support/SSEHelper.h"

#include "jimi/support/bitscan_reverse.h"
#include "jimi/support/bitscan_forward.h"

#define USE_SSE42_STRING_COMPARE    1

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

template <typename CharTy>
bool string_compare(const CharTy * str1, const CharTy * str2, size_t length)
{
    assert(str1 != nullptr && str2 != nullptr);

    static const int kMaxSize = jimi::SSEHelper<CharTy>::kMaxSize;
    static const int _SIDD_CHAR_OPS = jimi::SSEHelper<CharTy>::_SIDD_CHAR_OPS;
    static const int kEqualEach = _SIDD_CHAR_OPS | _SIDD_CMP_EQUAL_EACH
                                | _SIDD_NEGATIVE_POLARITY | _SIDD_LEAST_SIGNIFICANT;

#if 1
    if (likely(length > 0)) {
        ssize_t nlength = (ssize_t)length;
        do {
            __m128i __str1 = _mm_loadu_si128((const __m128i *)str1);
            __m128i __str2 = _mm_loadu_si128((const __m128i *)str2);
            int len = ((int)nlength >= kMaxSize) ? kMaxSize : (int)nlength;
            assert(len > 0);
            
            int full_matched = _mm_cmpestrc(__str1, len, __str2, len, kEqualEach);
            if (likely(full_matched == 0)) {
                // Full matched, continue match next kMaxSize bytes.
                str1 += kMaxSize;
                str2 += kMaxSize;
                nlength -= kMaxSize;
            }
            else {
                // It's dismatched.
                return false;
            }
        } while (nlength > 0);
    }
#else
    if (likely(length > 0)) {
        int nlength = (int)length;
        do {
            __m128i __str1 = _mm_loadu_si128((const __m128i *)str1);
            __m128i __str2 = _mm_loadu_si128((const __m128i *)str2);
            int len = (nlength >= kMaxSize) ? kMaxSize : nlength;
            assert(len > 0);
            
            int full_matched = _mm_cmpestrc(__str1, len, __str2, len, kEqualEach);
            if (likely(full_matched == 0)) {
                // Full matched, continue match next kMaxSize bytes.
                str1 += kMaxSize;
                str2 += kMaxSize;
                nlength -= kMaxSize;
            }
            else {
                // It's dismatched.
                return false;
            }
        } while (nlength > 0);
    }
#endif

    // It's matched, or the length is equal 0, .
    return true;
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

    size_type size() const { return this->size_; }
    size_type mask() const { return this->mask_; }
    size_type buckets() const { return this->buckets_; }
    data_type * data() const { return this->table_; }

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

    void clear() {
        this->destroy();
    }

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

    void reserve_internal(size_type new_buckets) {
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

    void inline rehash_insert(data_type * new_table, size_type new_buckets,
                              data_type old_data) {
        assert(new_table != nullptr);
        assert(old_data != nullptr);
        assert(new_buckets > 1);
        size_type new_mask = new_buckets - 1;

        const std::string & key = old_data->pair.first;
        hash_type hash = jimi::crc32_x64(key.c_str(), key.size());
        hash_type bucket = hash & new_mask;

        // Update the hash value
        old_data->hash = hash;

        if (likely(new_table[bucket] == nullptr)) {
            new_table[bucket] = old_data;
        }
        else {
            do {
                bucket = (bucket + 1) & new_mask;
                if (likely(new_table[bucket] == nullptr)) {
                    new_table[bucket] = old_data;
                    break;
                }
            } while (1);
        }
    }

    void rehash_internal(size_type new_buckets) {
        assert(new_buckets > 0);
        assert((new_buckets & (new_buckets - 1)) == 0);
        if (likely(new_buckets > this->buckets_)) {
            data_type * new_table = new data_type[new_buckets];
            if (new_table != nullptr) {
                // Initialize new table.
                memset(new_table, 0, sizeof(data_type) * new_buckets);
                if (likely(this->table_ != nullptr)) {
                    // Recalculate all hash values.
                    {
                        size_type new_size = 0;

                        for (size_type i = 0; i < this->buckets_; ++i) {
                            if (this->table_[i] != nullptr) {
                                // Insert the old buckets to the new buckets in the new table.
                                this->rehash_insert(new_table, new_buckets, this->table_[i]);
                                ++new_size;
                            }
                        }
                        assert(new_size == this->size_);
                    }

                    // Free old table data.
                    delete[] this->table_;
                }
                this->table_ = new_table;
                this->mask_ = new_buckets - 1;
                this->buckets_ = new_buckets;
            }
        }
    }

    void resize_internal(size_type new_buckets) {
        assert(new_buckets > 0);
        assert((new_buckets & (new_buckets - 1)) == 0);
        rehash_internal(new_buckets);
    }

public:
    void reserve(size_type new_buckets) {
        new_buckets = (new_buckets >= kBucketsInit) ? (new_buckets - 1) : (kBucketsInit - 1);
        size_type new_capacity = detail::round_up_pow2(new_buckets);
        this->reserve_internal(new_capacity);
    }

    void resize(size_type new_buckets) {
        new_buckets = (new_buckets >= kBucketsInit) ? (new_buckets - 1) : (kBucketsInit - 1);
        size_type new_capacity = detail::round_up_pow2(new_buckets);
        this->resize_internal(new_capacity);
    }

    iterator find(const key_type & key) {
        hash_type hash = jimi::crc32_x64(key.c_str(), key.size());
        hash_type bucket = hash & this->mask_;
        node_type * node = (node_type *)this->table_[bucket];
#if 1
        if (likely(node != nullptr)) {
            // Found, next to check the hash value.
            if (likely(node->hash == hash)) {
                // If hash value is equal, then compare the key sizes and the strings.
                if (likely(node->pair.first.size() == key.size())) {
#if USE_SSE42_STRING_COMPARE
                    if (likely(detail::string_compare(node->pair.first.c_str(), key.c_str(), key.size()))) {
                        return (iterator)&this->table_[bucket];
                    }
#else
                    if (likely(strcmp(node->pair.first.c_str(), key.c_str()) == 0)) {
                        return (iterator)&this->table_[bucket];
                    }
#endif
                }
            }

            // If first position is not found, search next bucket continue.
            hash_type first_bucket = bucket;
            do {
                bucket = (bucket + 1) & this->mask_;
                node = (node_type *)this->table_[bucket];
                if (likely(node != nullptr)) {
                    if (likely(node->hash == hash)) {
                        // If hash value is equal, then compare the key sizes and the strings.
                        if (likely(node->pair.first.size() == key.size())) {
#if USE_SSE42_STRING_COMPARE
                            if (likely(detail::string_compare(node->pair.first.c_str(), key.c_str(), key.size()))) {
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
        }
#else
        if (likely(node != nullptr)) {
            // Found, next to check the hash value.
            if (likely((node->hash + node->pair.first.size()) == (hash + key.size()))) {
                // If hash value and key sizes is equal, then compare the strings.
                if (likely(strcmp(node->pair.first.c_str(), key.c_str()) == 0)) {
                    return (iterator)&this->table_[bucket];
                }
            }

            // If first position is not found, search next bucket continue.
            hash_type first_bucket = bucket;
            do {
                bucket = (bucket + 1) & this->mask_;
                node = (node_type *)this->table_[bucket];
                if (likely(node != nullptr)) {
                    if (likely((node->hash + node->pair.first.size()) == (hash + key.size()))) {
                        // If hash value and key sizes is equal, then compare the strings.
                        if (likely(strcmp(node->pair.first.c_str(), key.c_str()) == 0)) {
                            return (iterator)&this->table_[bucket];
                        }
                    }
                }
            } while (likely(bucket != first_bucket));
        }
#endif
        // Not found
        return this->end();
    }

    void insert(const key_type & key, const value_type & value) {
        iterator iter = this->find(key);
        if (likely(iter == this->end())) {
            // Insert the new key.
            if (unlikely(this->size_ >= (this->buckets_ * 3 / 4))) {
                this->resize_internal(this->buckets_ * 2);
            }

            node_type * new_data = new node_type(key, value);
            if (likely(new_data != nullptr)) {
                hash_type hash = jimi::crc32_x64(key.c_str(), key.size());
                hash_type bucket = hash & this->mask_;
                new_data->hash = hash;
                if (likely(this->table_[bucket] == nullptr)) {
                    this->table_[bucket] = (data_type)new_data;
                    ++(this->size_);
                }
                else {
                    do {
                        bucket = (bucket + 1) & this->mask_;
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
            if (unlikely(this->size_ >= (this->buckets_ * 3 / 4))) {
                this->resize_internal(this->buckets_ * 2);
            }

            node_type * new_data = new node_type(std::forward<key_type>(key),
                                                 std::forward<value_type>(value));
            if (likely(new_data != nullptr)) {
                hash_type hash = jimi::crc32_x64(key.c_str(), key.size());
                hash_type bucket = hash & this->mask_;
                new_data->hash = hash;
                if (likely(this->table_[bucket] == nullptr)) {
                    this->table_[bucket] = (data_type)new_data;
                    ++(this->size_);
                }
                else {
                    do {
                        bucket = (bucket + 1) & this->mask_;
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
};

} // namespace jstd

#endif // JSTD_HASH_TABLE_H
