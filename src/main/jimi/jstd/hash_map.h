
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

#define TEST_JSTD_HASH_MAP          1

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
    hash_map_list(entry_type * entry) : head_(entry), size_((entry != nullptr) ? 1 : 0) {}
    hash_map_list(const this_type & src) : head_(src.head_), size_(src.size_) {}
    hash_map_list(this_type && src) {
        this->head_ = src.head_;
        this->size_ = src.size_;
        src.head_ = nullptr;
        src.size_ = 0;
    }
    ~hash_map_list() {
        this->destroy();
    }

    entry_type * head() const { return this->head_; }
    size_type size() const { return this->size_; }

private:
    void destroy() {
        entry_type * head = this->head_;
        if (likely(head != nullptr)) {
            entry_type * entry = head->next;
            delete head;
            while (likely(entry != nullptr)) {
                entry_type * next = entry->next;
                delete entry;
                entry = next;
            }
            this->head_ = nullptr;
#ifndef NDEBUG
            this->size_ = 0;
#endif
        }
    }

public:
    entry_type * front() const { return this->head(); }
    entry_type * back() const {
        entry_type * entry = this->head_;
        while (likely(entry != nullptr)) {
            if (likely(entry->next != nullptr))
                entry = entry->next;
            else
                return entry;
        }
        return nullptr;
    }

    void reset() {
        this->head_ = nullptr;
#ifndef NDEBUG
        this->size_ = 0;
#endif
    }

    void clear() {
        this->destroy();
#ifdef NDEBUG
        this->size_ = 0;
#endif
    }

    void push_front(entry_type * entry) {
        assert(entry != nullptr);
        if (likely(this->head_ != nullptr)) {
            entry->next = this->head_;
        }
        this->head_ = entry;
        ++(this->size_);
    }

    void pop_front() {
        entry_type * entry = this->head_;
        if (likely(entry != nullptr)) {
            this->head_ = entry->next;
            delete entry;
            --(this->size_);
        }
    }

    void erase(entry_type * before) {
        if (likely(before != nullptr)) {
            entry_type * entry = this->head_;
            while (likely(entry != nullptr)) {
                if (likely(entry != before)) {
                    // It's not before
                    if (likely(entry->next != nullptr))
                        entry = entry->next;
                    else
                        return;
                }
                else {
                    // Current entry is before
                    if (likely(entry->next != nullptr)) {
                        entry_type * target = entry->next;
                        entry->next = target->next;
                        delete target;
                        --(this->size_);
                    }
                    else {
                        // Error: no entry after [before]
                    }
                    break;
                }
            }
        }
        else {
            pop_front();
        }
    }

    void swap(const this_type & src) {
        entry_type * head_save = src.head_;
        size_type size_save = src.size_;
        src.head_ = this->head_;
        src.size_ = this->size_;
        this->head_ = head_save;
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

    // Default initial capacity is 64.
    static const size_type kDefaultInitialCapacity = 64;
    // Maximum capacity is 1 << 30.
    static const size_type kMaximumCapacity = 1U << 30;
    // Default load factor is: 0.75
    static const float kDefaultLoadFactor;

public:
    basic_hash_map(size_type initialCapacity = kDefaultInitialCapacity,
                   float loadFactor = kDefaultLoadFactor)
        : table_(nullptr), capacity_(0), size_(0), used_(0),
          threshold_(0), loadFactor_(loadFactor) {
        this->init(initialCapacity, loadFactor);
    }
    ~basic_hash_map() {
        this->destroy();
    }

    bool is_valid() const { return (this->table_ != nullptr); }

    iterator begin() const { return this->table_[0]->head(); }
    iterator end() const { return nullptr; }

    size_type size() const { return this->size_; }
    size_type bucket_used() const { return this->used_; }
    size_type bucket_count() const { return this->capacity_; }
    list_type * data() const { return this->table_; }

    bool empty() const { return (this->size() == 0); }

private:
    void init(size_type new_capacity, float loadFactor) {
        assert(new_capacity > 0);
        assert((new_capacity & (new_capacity - 1)) == 0);
        assert(loadFactor > 0.0f);
        list_type ** new_table = new list_type *[new_capacity];
        if (likely(new_table != nullptr)) {
            // Reset the table data.
            memset(new_table, 0, sizeof(list_type *) * new_capacity);
            // Setting status
            this->table_ = new_table;
            this->capacity_ = new_capacity;
            this->size_ = 0;
            this->used_ = 0;
            this->threshold_ = (size_type)(new_capacity * abs(loadFactor));
        }
    }

    void destroy() {
#ifdef NDEBUG
        // Clear all data, and free the table.
        if (likely(this->table_ != nullptr)) {
            for (size_type i = 0; i < this->capacity_; ++i) {
                list_type * list = (list_type *)this->table_[i];
                if (likely(list != nullptr)) {
                    delete list;
                }
            }
            delete[] this->table_;
            this->table_ = nullptr;
        }
#else
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
        // Setting status
        this->capacity_ = 0;
        this->size_ = 0;
        this->used_ = 0;
        this->threshold_ = 0;
#endif
    }

#if TEST_JSTD_HASH_MAP
    inline size_type calc_capacity(size_type new_capacity) {
        // If new_capacity is less than half of the current hash table size,
        // then double the hash table size.
        new_capacity = (new_capacity > (this->size_ * 2)) ? new_capacity : (this->size_ * 2);
        // The minimum bucket is kDefaultInitialCapacity = 64.
        new_capacity = (new_capacity >= kDefaultInitialCapacity) ? new_capacity : kDefaultInitialCapacity;
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

    static inline
    size_type index_for(hash_type hash, size_type capacity) {
#if 0
        size_type index = ((size_type)hash % capacity);
#else
        size_type index = ((size_type)hash & (capacity - 1));
#endif
        return index;
    }

    static inline
    size_type next_index(size_type index, size_type capacity) {
#if 0
        index = ((index + 1) % capacity);
#else
        index = ((index + 1) & (capacity - 1));
#endif
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
                // Setting status
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

        entry_type * old_entry = old_list->head();
        while (likely(old_entry != nullptr)) {
            entry_type * next_entry;
            hash_type hash = old_entry->hash;
            size_type index = this_type::index_for(hash, new_capacity);

            list_type * list = new_table[index];
            if (likely(list == nullptr)) {
                // Create the new list and push the new entry to front of new list.
                list_type * new_list = new list_type(old_entry);
                if (likely(new_list != nullptr)) {
                    list = new_list;
                    assert(new_table[index] == nullptr);
                    new_table[index] = new_list;
                    // Save the value of old_entry->next.
                    next_entry = old_entry->next;
                    // Modify the value of old_entry->next.
                    old_entry->next = nullptr;
                    ++(this->size_);
                    ++(this->used_);
                }
            }
            else {
                // Push the new entry to front of old list.
                assert(list != nullptr);
                entry_type * front = list->front();
                // Save the value of old_entry->next.
                next_entry = old_entry->next;
                // list->push_front(old_entry) will modify the value of old_entry->next.
                list->push_front(old_entry);
                ++(this->size_);
                if (unlikely(front == nullptr)) {
                    ++(this->used_);
                }
            }
            // Scan next entry
            old_entry = next_entry;
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
                    size_type old_used = this->used_;
                    size_type old_size = this->size_;

                    this->used_ = 0;
                    this->size_ = 0;

                    for (size_type i = 0; i < this->capacity_; ++i) {
                        list_type * old_list = this->table_[i];
                        if (likely(old_list != nullptr)) {
                            // Insert the old buckets to the new buckets in the new table.
                            this->reinsert_list(new_table, new_capacity, old_list);
                            // Set the old_list->head to nullptr.
                            old_list->reset();
                        }
                    }
                    assert(this->size_ == old_size);

                    // Free old table data.
                    delete[] this->table_;
                }
                // Setting status
                this->table_ = new_table;
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
                    size_type old_used = this->used_;
                    size_type old_size = this->size_;

                    this->used_ = 0;
                    this->size_ = 0;

                    for (size_type i = 0; i < this->capacity_; ++i) {
                        list_type * old_list = this->table_[i];
                        if (likely(old_list != nullptr)) {
                            // Insert the old buckets to the new buckets in the new table.
                            this->reinsert_list(new_table, new_capacity, old_list);
                            // Set the old_list->head to nullptr.
                            old_list->reset();
                        }
                    }
                    assert(this->size_ == old_size);

                    // Free old table data.
                    delete[] this->table_;
                }
                // Setting status
                this->table_ = new_table;
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
        index = this_type::index_for(hash, this->capacity_);

        assert(this->table_ != nullptr);
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
    void clear() {
        // Clear the data only, don't free the table.
        if (likely(this->table_ != nullptr)) {
            for (size_type i = 0; i < this->capacity_; ++i) {
                list_type * list = (list_type *)this->table_[i];
                if (likely(list != nullptr)) {
                    list->clear();
                    //delete list;
                    //this->table_[i] = nullptr;
                }
            }
        }
        // Setting status
        this->size_ = 0;
        this->used_ = 0;
    }

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
        size_type index = this_type::index_for(hash, this->capacity_);

        if (likely(this->table_ != nullptr)) {
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
        }

        // Not found
        return this->end();
    }

    void insert(const key_type & key, const value_type & value) {
        if (likely(this->table_ != nullptr)) {
            hash_type hash;
            size_type index;
            iterator iter = this->find_internal(key, hash, index);
            if (likely(iter == this->end())) {
                // Insert the new key.
                if (likely(this->size_ >= this->threshold_)) {
                    // Resize the table
                    this->resize_internal(this->capacity_ * 2);
                    // Recalculate the index.
                    index = this_type::index_for(hash, this->capacity_);
                }

                entry_type * new_entry = new entry_type(hash, key, value);
                if (likely(new_entry != nullptr)) {
                    list_type * list = this->table_[index];
                    if (likely(list == nullptr)) {
                        // Create new list and push the new entry to front of new list.
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
                        // Push the new entry to front of old list.
                        assert(list != nullptr);
                        entry_type * front = list->front();
                        list->push_front(new_entry);
                        ++(this->size_);
                        if (unlikely(front == nullptr)) {
                            ++(this->used_);
                        }
                    }
                }
            }
            else {
                // Update the existed key's value.
                assert(iter != nullptr);
                iter->pair.second = value;
            }
        }
    }

    void insert(key_type && key, value_type && value) {
        if (likely(this->table_ != nullptr)) {
            hash_type hash;
            size_type index;
            iterator iter = this->find_internal(std::forward<key_type>(key), hash, index);
            if (likely(iter == this->end())) {
                // Insert the new key.
                if (likely(this->size_ >= this->threshold_)) {
                    // Resize the table
                    this->resize_internal(this->capacity_ * 2);
                    // Recalculate the index.
                    index = this_type::index_for(hash, this->capacity_);
                }

                entry_type * new_entry = new entry_type(hash,
                                                        std::forward<key_type>(key),
                                                        std::forward<value_type>(value));
                if (likely(new_entry != nullptr)) {
                    list_type * list = this->table_[index];
                    if (likely(list == nullptr)) {
                        // Create new list and push the new entry to front of new list.
                        list_type * new_list = new list_type(new_entry);
                        if (likely(new_list != nullptr)) {
                            assert(this->table_[index] == nullptr);
                            this->table_[index] = new_list;
                            ++(this->size_);
                            ++(this->used_);
                        }
                    }
                    else {
                        // Push the new entry to front of old list.
                        assert(list != nullptr);
                        entry_type * front = list->front();
                        list->push_front(new_entry);
                        ++(this->size_);
                        if (unlikely(front == nullptr)) {
                            ++(this->used_);
                        }
                    }
                }
            }
            else {
                // Update the existed key's value.
                assert(iter != nullptr);
                iter->pair.second = std::move(std::forward<value_type>(value));
            }
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
        if (likely(this->table_ != nullptr)) {
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
    }

    void erase(key_type && key) {
        if (likely(this->table_ != nullptr)) {
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
    }
#endif // TEST_JSTD_HASH_MAP

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

template <typename Key, typename Value, std::size_t HashFunc>
const float basic_hash_map<Key, Value, HashFunc>::kDefaultLoadFactor = 0.75f;

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
