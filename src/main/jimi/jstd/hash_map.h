
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
          pair(std::forward<key_type>(key), std::forward<value_type>(value)) {}

    hash_map_entry(const key_type & key, const value_type & value)
        : hash(0), next(nullptr), pair(key, value) {}
    hash_map_entry(key_type && key, value_type && value)
        : hash(0), next(nullptr),
          pair(std::forward<key_type>(key), std::forward<value_type>(value)) {}

    ~hash_map_entry() {}
};

template <typename Key, typename Value>
class hash_map_list {
public:
    typedef hash_map_entry<Key, Value>      entry_type;
    typedef typename entry_type::size_type  size_type;
    typedef hash_map_list<Key, Value>       this_type;

private:
    entry_type * head_;
    size_type    size_;

public:
    hash_map_list() : head_(nullptr), size_(0) {}
    hash_map_list(entry_type * next) : head_(next), size_((next != nullptr) ? 1 : 0) {}
    hash_map_list(const this_type & src) : head_(src.next_), size_(src.size_) {}
    hash_map_list(this_type && src) {
        this->head_ = src.next_;
        this->size_ = src.size_;
        src.next_ = nullptr;
        src.size_ = 0;
    }
    ~hash_map_list() {
        this->destroy();
    }

    entry_type * head() const { return this->head_; }
    size_type size() const { return this->size_; }

    void destroy() {
        entry_type * entry = this->head_;
        while (likely(entry != nullptr)) {
            entry_type * next = entry->next;
            delete entry;
            entry = next;
        }
        this->head_ = nullptr;
        this->size_ = 0;
    }

    void push_back(entry_type * entry) {
        assert(entry != nullptr);
        if (likely(this->head_ != nullptr)) {
            entry->next = this->head_;
        }
        this->head_ = entry;
        ++(this->size_);
    }

    void swap(const this_type & src) {
        entry_type * next_save = src.next_;
        size_type size_save = src.size_;
        src.next_ = this->head_;
        src.size_ = this->size_;
        this->head_ = next_save;  
        this->size_ = size_save;
    }
};

template <typename Key, typename Value, std::size_t HashFunc = Hash_CRC32C>
class basic_hash_map {
public:
    typedef Key                                     key_type;
    typedef Value                                   value_type;
    typedef std::pair<Key, Value>                   pair_type;
    typedef std::size_t                             size_type;
    typedef std::uint32_t                           hash_type;

    typedef hash_map_entry<Key, Value>              entry_type;
    typedef hash_map_list<Key, Value>               list_type;
    typedef entry_type *                            iterator;
    typedef const entry_type *                      const_iterator;
    typedef basic_hash_map<Key, Value, HashFunc>    this_type;

private:
    list_type ** table_;
    size_type capacity_;
    size_type size_;
    size_type used_;
    size_type threshold_;
    float loadFactor_;

    // Default capacity is 64.
    static const size_type kDefaultCapacity = 64;
    // Default load factor is: 0.75
    const float kDefaultLoadFactor = 0.75f;

public:
    basic_hash_map(size_type initialCapacity = kDefaultCapacity)
        : table_(nullptr), capacity_(0), size_(0), used_(0),
          threshold_(0), loadFactor_(kDefaultLoadFactor) {
        this->init(initialCapacity);
    }
    ~basic_hash_map() {
        this->destroy();
    }

    iterator begin() const { return this->table_[0]->head(); }
    iterator end() const { return nullptr; }

    size_type size() const { return this->size_; }
    size_type bucket_used() const { return this->used_; }
    size_type bucket_count() const { return this->capacity_; }
    list_type * data() const { return this->table_; }

    bool empty() const { return (this->size() == 0); }

    void destroy() {
        // Clear all data, and free the table.
        if (likely(this->table_ != nullptr)) {
            for (size_type i = 0; i < this->capacity_; ++i) {
                list_type * list = (list_type *)this->table_[i];
                if (likely(list != nullptr)) {
                    delete list;
                    this->table_[i] = nullptr;
                }
            }
            delete[] this->table_;
            this->table_ = nullptr;
        }
        // Setting
        this->capacity_ = 0;
        this->size_ = 0;
        this->used_ = 0;
        this->threshold_ = 0;
    }

    void clear() {
        // Clear the data only, don't free the table.
        if (likely(this->table_ != nullptr)) {
            for (size_type i = 0; i < this->capacity_; ++i) {
                list_type * list = (list_type *)this->table_[i];
                if (likely(list != nullptr)) {
                    delete list;
                    this->table_[i] = nullptr;
                }
            }
        }
        // Setting
        this->size_ = 0;
        this->used_ = 0;
    }

private:
    void init(size_type new_capacity) {
        assert(new_capacity > 0);
        assert((new_capacity & (new_capacity - 1)) == 0);
        list_type ** new_table = new list_type *[new_capacity];
        if (new_table != nullptr) {
            // Reset the table data.
            memset(new_table, 0, sizeof(list_type *) * new_capacity);
            // Setting
            this->table_ = new_table;
            this->capacity_ = new_capacity;
            this->size_ = 0;
            this->used_ = 0;
            this->threshold_ = (size_type)(new_capacity * this->loadFactor_);       
        }
    }

#if 0
    inline size_type calc_capacity(size_type new_capacity) {
        // If new_capacity is less than half of the current hash table size,
        // then double the hash table size.
        new_capacity = (new_capacity > (this->size_ * 2)) ? new_capacity : (this->size_ * 2);
        // The minimum bucket is kBucketsInit = 64.
        new_capacity = (new_capacity >= kDefaultCapacity) ? new_capacity : kDefaultCapacity;
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

    inline size_type index_for(hash_type hash, size_type capacity) {
        size_type index = ((size_type)hash % capacity);
        return index;
    }

    inline size_type next_index(size_type index, size_type capacity) {
        index = ((index + 1) % capacity);
        return index;
    }

    void reserve_internal(size_type new_capacity) {
        assert(new_capacity > 0);
        assert((new_capacity & (new_capacity - 1)) == 0);
        if (likely(new_capacity > this->capacity_)) {
            list_type ** new_table = new list_type *[new_capacity];
            if (new_table != nullptr) {
                // Reset the table data.
                memset(new_table, 0, sizeof(list_type *) * new_capacity);
                if (likely(this->table_ != nullptr)) {
                    delete[] this->table_;
                }
                // Setting
                this->used_ = 0;
                this->size_ = 0;
                this->capacity_ = new_capacity;
                this->table_ = new_table;
            }
        }
    }

    inline void reinsert_list(list_type ** new_table, size_type new_capacity,
                              list_type * old_list) {
        assert(new_table != nullptr);
        assert(old_list != nullptr);
        assert(new_capacity > 1);

        const std::string & key = old_list->pair.first;
        hash_type hash = hash_helper<HashFunc>::getHash(key.c_str(), key.size());
        size_type index = this->index_for(hash, new_capacity);

        // Update the hash value
        old_list->hash = hash;

        if (likely(new_table[index] == nullptr)) {
            new_table[index] = old_list;
        }
        else {
            do {
                index = this->next_index(index, new_capacity);
                if (likely(new_table[index] == nullptr)) {
                    new_table[index] = old_list;
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
            list_type ** new_table = new list_type *[new_capacity];
            if (likely(new_table != nullptr)) {
                // Reset the new table data.
                memset(new_table, 0, sizeof(list_type *) * new_capacity);

                if (likely(this->table_ != nullptr)) {
                    // Recalculate all hash values.
                    size_type new_size = 0;

                    for (size_type i = 0; i < this->capacity_; ++i) {
                        if (likely(this->table_[i] != nullptr)) {
                            // Insert the old buckets to the new buckets in the new table.
                            this->reinsert_list(new_table, new_capacity, this->table_[i]);
                            ++new_size;
                        }
                    }
                    assert(new_size == this->size_);

                    // Free old table data.
                    delete[] this->table_;
                }
                // Setting
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
            list_type ** new_table = new list_type *[new_capacity];
            if (likely(new_table != nullptr)) {
                // Reset the new table data.
                memset(new_table, 0, sizeof(list_type *) * new_capacity);

                if (likely(this->table_ != nullptr)) {
                    // Recalculate all hash values.
                    size_type new_size = 0;

                    for (size_type i = 0; i < this->capacity_; ++i) {
                        if (likely(this->table_[i] != nullptr)) {
                            // Insert the old buckets to the new buckets in the new table.
                            this->reinsert_list(new_table, new_capacity, this->table_[i]);
                            ++new_size;
                        }
                    }
                    assert(new_size == this->size_);

                    // Free old table data.
                    delete[] this->table_;
                }
                // Setting
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

    iterator find_internal(const key_type & key, hash_type & hash, size_type & index) {
        hash = hash_helper<HashFunc>::getHash(key.c_str(), key.size());
        index = this->index_for(hash, this->capacity_);

        list_type * list = (list_type *)this->table_[index];
        if (likely(list != nullptr)) {
            entry_type * entry = list->head();
            while (likely(entry != nullptr)) {
                // Found entry, next to check the hash value.
                if (likely(entry->hash == hash)) {
                    // If hash value is equal, then compare the key sizes and the strings.
                    if (likely(entry->pair.first.size() == key.size())) {
#if USE_SSE42_STRING_COMPARE
                        if (likely(detail::string_equal(entry->pair.first.c_str(), key.c_str(), key.size()))) {
                            return (iterator)entry;
                        }
#else
                        if (likely(strcmp(entry->pair.first.c_str(), key.c_str()) == 0)) {
                            return (iterator)entry;
                        }
#endif
                    }
                }
                // Scan next entry
                entry = entry->next;
            }
        }

        // Not found
        return this->end();
    }

public:
    void reserve(size_type new_capacity) {
        // Recalculate the size of new_capacity.
        new_capacity = this->calc_capacity(new_capacity);
        this->reserve_internal(new_capacity);
    }

    void rehash(size_type new_capacity) {
        // Recalculate the size of new_capacity.
        new_capacity = this->calc_capacity(new_capacity);
        this->rehash_internal(new_capacity);
    }

    void resize(size_type new_capacity) {
        this->rehash(new_capacity);
    }

    void shrink_to(size_type new_capacity) {
        // Recalculate the size of new_capacity.
        new_capacity = this->calc_capacity_fast(new_capacity);
        this->shrink_internal(new_capacity);
    }

    iterator find(const key_type & key) {
        hash_type hash = hash_helper<HashFunc>::getHash(key.c_str(), key.size());
        size_type index = this->index_for(hash, this->capacity_);
        list_type * list = (list_type *)this->table_[index];

        if (likely(list != nullptr)) {
            entry_type * entry = list->head();
            while (likely(entry != nullptr)) {
                // Found entry, next to check the hash value.
                if (likely(entry->hash == hash)) {
                    // If hash value is equal, then compare the key sizes and the strings.
                    if (likely(entry->pair.first.size() == key.size())) {
#if USE_SSE42_STRING_COMPARE
                        if (likely(detail::string_equal(entry->pair.first.c_str(), key.c_str(), key.size()))) {
                            return (iterator)&this->table_[index];
                        }
#else
                        if (likely(strcmp(entry->pair.first.c_str(), key.c_str()) == 0)) {
                            return (iterator)&this->table_[index];
                        }
#endif
                    }
                }
                // Scan next entry
                entry = entry->next;
            }
        }

        // Not found
        return this->end();
    }

    void insert(const key_type & key, const value_type & value) {
        hash_type hash;
        size_type index;
        iterator iter = this->find_internal(key, hash, index);
        if (likely(iter == this->end())) {
            // Insert the new key.
            if (unlikely(this->size_ >= this->threshold_)) {
                // Resize the table
                this->resize_internal(this->capacity_ * 2);
                // Recalculate the index.
                index = this->index_for(hash, this->capacity_);
            }
            
            entry_type * new_entry = new entry_type(hash, key, value);
            if (likely(new_entry != nullptr)) {
                list_type * list = this->table_[index];
                if (likely(list == nullptr)) {
                    // Insert the new list and push the new entry to new list.
                    list_type * new_list = new list_type(new_entry);
                    if (likely(new_list != nullptr)) {
                        list = new_list;
                        assert(this->table_[index] == nullptr);
                        this->table_[index] = new_list;
                        ++(this->size_);
                        ++(this->used_);
                    }
                }
                else {
                    // Push the new entry to list back.
                    assert(list != nullptr);
                    list->push_back(new_entry);
                    ++(this->size_);
                }
            }
        }
        else {
            // Update the existed key's value.
            assert(iter != nullptr);
            iter->pair.second = value;
        }
    }

    void insert(key_type && key, value_type && value) {
        hash_type hash;
        size_type index;
        iterator iter = this->find_internal(std::forward<key_type>(key), hash, index);
        if (likely(iter == this->end())) {
            // Insert the new key.
            if (unlikely(this->size_ >= this->threshold_)) {
                // Resize the table
                this->resize_internal(this->capacity_ * 2);
                // Recalculate the index.
                index = this->index_for(hash, this->capacity_);
            }
            
            entry_type * new_entry = new entry_type(hash,
                                                    std::forward<key_type>(key),
                                                    std::forward<value_type>(value));
            if (likely(new_entry != nullptr)) {
                list_type * list = this->table_[index];
                if (likely(list == nullptr)) {
                    // Insert the new list and push the new entry to new list.
                    list_type * new_list = new list_type(new_entry);
                    if (likely(new_list != nullptr)) {
                        list = new_list;
                        assert(this->table_[index] == nullptr);
                        this->table_[index] = new_list;
                        ++(this->size_);
                        ++(this->used_);
                    }
                }
                else {
                    // Push the new entry to list back.
                    assert(list != nullptr);
                    list->push_back(new_entry);
                    ++(this->size_);
                }
            }
        }
        else {
            // Update the existed key's value.
            assert(iter != nullptr);
            iter->pair.second = std::move(std::forward<value_type>(value));
        }
    }

    void insert(const pair_type & pair) {
        this->insert(pair.first, pair.second);
    }

    void insert(pair_type && pair) {
        this->insert(std::forward<key_type>(pair.first), std::forward<value_type>(pair.second));
    }

    void emplace(const pair_type & pair) {
        this->insert(pair.first, pair.second);
    }

    void emplace(pair_type && pair) {
        this->insert(std::forward<key_type>(pair.first), std::forward<value_type>(pair.second));
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
#endif

    static const char * name() {
        switch (HashFunc) {
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
