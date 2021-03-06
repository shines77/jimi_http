
#ifndef JSTD_HASH_MAP_H
#define JSTD_HASH_MAP_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "jimi/basic/stddef.h"
#include "jimi/basic/stdint.h"

#include <memory.h>
#include <assert.h>

#include <cstddef>
#include <memory>
#include <cmath>
#include <type_traits>

#include "jimi/jstd/hash_helper.h"
#include "jimi/jstd/string_utils.h"
#include "jimi/jstd/nothrow_new.h"

#include "jimi/support/Power2.h"

#define USE_JSTD_HASH_MAP           1

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

    this_type * next;
    hash_type   hash;
    pair_type   pair;

    hash_map_entry() : next(nullptr), hash(0) {}
    hash_map_entry(hash_type hash_code) : next(nullptr), hash(hash_code) {}

    hash_map_entry(hash_type hash_code, const key_type & key,
                   const value_type & value, this_type * next_entry = nullptr)
        : next(next_entry), hash(hash_code), pair(key, value) {}
    hash_map_entry(hash_type hash_code, key_type && key,
                   value_type && value, this_type * next_entry = nullptr)
        : next(next_entry), hash(hash_code),
          pair(std::forward<key_type>(key), std::forward<value_type>(value)) {}

    hash_map_entry(const key_type & key, const value_type & value)
        : next(nullptr), hash(0), pair(key, value) {}
    hash_map_entry(key_type && key, value_type && value)
        : next(nullptr), hash(0),
          pair(std::forward<key_type>(key), std::forward<value_type>(value)) {}

    ~hash_map_entry() {
#ifndef NDEBUG
        this->next = nullptr;
#endif
    }
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

    void set_head(entry_type * entry) {
        this->head_ = entry;
    }

    void set_size(size_type size) {
        this->size_ = size;
    }

private:
    void destroy() {
        entry_type * head = this->head_;
        if (likely(head != nullptr)) {
            entry_type * entry = head->next;
            //delete head;
            jstd::nothrow_deleter::destroy(head);
            while (likely(entry != nullptr)) {
                entry_type * next = entry->next;
                //delete entry;
                jstd::nothrow_deleter::destroy(entry);
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
        this->size_ = 0;
    }

    void push_first(entry_type * entry) {
        assert(entry != nullptr);
        assert(this->head_ == nullptr);
        this->head_ = entry;
        entry->next = nullptr;
        assert(this->size_ == 0);
        this->size_ = 1;
    }

    void push_first_fast(entry_type * entry) {
        assert(entry != nullptr);
        assert(this->head_ == nullptr);
        this->head_ = entry;
        assert(entry->next == nullptr);
        assert(this->size_ == 0);
        this->size_ = 1;
    }

    void push_front(entry_type * entry) {
        assert(entry != nullptr);
        if (likely(this->head_ != nullptr)) {
            entry->next = this->head_;
        }
        this->head_ = entry;
        assert(this->size_ >= 1);
        ++(this->size_);
    }

    void push_front_fast(entry_type * entry) {
        assert(entry != nullptr);
        assert(this->head_ != nullptr);
        entry->next = this->head_;
        this->head_ = entry;
        assert(this->size_ >= 1);
        ++(this->size_);
    }

    void pop_front() {
        entry_type * entry = this->head_;
        if (likely(entry != nullptr)) {
            this->head_ = entry->next;
            //delete entry;
            jstd::nothrow_deleter::destroy(entry);
            assert(this->size_ > 0);
            --(this->size_);
        }
    }

    void pop_front_fast() {
        entry_type * entry = this->head_;
        assert(entry != nullptr);
        this->head_ = entry->next;
        //delete entry;
        jstd::nothrow_deleter::destroy(entry);
        assert(this->size_ > 0);
        --(this->size_);
    }

    void erase(entry_type * before) {
        if (likely(before != nullptr)) {
            entry_type * target = before->next;
            if (likely(target != nullptr)) {
                before->next = target->next;
                //delete target;
                jstd::nothrow_deleter::destroy(target);

                assert(this->size_ > 0);
                --(this->size_);
            }
        }
        else {
            entry_type * target = this->head_;
            if (likely(target != nullptr)) {
                this->head_ = target->next;
                //delete target;
                jstd::nothrow_deleter::destroy(target);

                assert(this->size_ > 0);
                --(this->size_);
            }
        }
    }

    void erase_fast(entry_type * before) {
        if (likely(before != nullptr)) {
            entry_type * target = before->next;
            assert(target != nullptr);
            before->next = target->next;
            //delete target;
            jstd::nothrow_deleter::destroy(target);

            assert(this->size_ > 0);
            --(this->size_);
        }
        else {
            entry_type * target = this->head_;
            assert(target != nullptr);
            this->head_ = target->next;
            //delete target;
            jstd::nothrow_deleter::destroy(target);

            assert(this->size_ > 0);
            --(this->size_);
        }
    }

    void erase_slow(entry_type * before) {
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
                        //delete target;
                        jstd::nothrow_deleter::destroy(target);
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

    void swap(this_type & right) {
        if (&right != this) {
            entry_type * head_save = right.head_;
            size_type size_save = right.size_;
            right.head_ = this->head_;
            right.size_ = this->size_;
            this->head_ = head_save;
            this->size_ = size_save;
        }
    }
};

template <typename Key, typename Value>
inline void swap(hash_map_list<Key, Value> & lhs,
                 hash_map_list<Key, Value> & rhs) {
    lhs.swap(rhs);
}

template <typename Key, typename Value, std::size_t HashFunc = HashFunc_Default>
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

#if defined(WIN64) || defined(_WIN64) || defined(_M_X64) || defined(_M_AMD64) \
 || defined(_M_IA64) || defined(__amd64__) || defined(__x86_64__) || defined(_M_ARM64)
    typedef float   float_type;
#else
    typedef float   float_type;
#endif

private:
    list_type ** table_;
    size_type    size_;
    size_type    mask_;
    size_type    capacity_;
    size_type    threshold_;
    float_type   loadFactor_;

    // Default initial capacity is 64.
    static const size_type kDefaultInitialCapacity = 64;
    // Maximum capacity is 1 << 30.
    static const size_type kMaximumCapacity = 1U << 30;
    // Default load factor is: 0.75
    static const float_type kDefaultLoadFactor;
    // The threshold of treeify to red-black tree.
    static const size_type kTreeifyThreshold = 8;

public:
    basic_hash_map(size_type initialCapacity = kDefaultInitialCapacity,
                   float_type loadFactor = kDefaultLoadFactor)
        : table_(nullptr), size_(0), mask_(0), capacity_(0),
          threshold_(0), loadFactor_(loadFactor) {
        this->initialize(initialCapacity, loadFactor);
    }
    ~basic_hash_map() {
        this->destroy();
    }

    iterator begin() const { return this->table_[0]->head(); }
    iterator end() const { return nullptr; }

    size_type size() const { return this->size_; }
    size_type bucket_used() const { return 0; }
    size_type bucket_mask() const { return this->mask_; }
    size_type bucket_count() const { return this->capacity_; }
    list_type * data() const { return this->table_; }

    bool is_valid() const { return (this->table_ != nullptr); }
    bool empty() const { return (this->size() == 0); }

private:
    void initialize(size_type new_capacity, float_type loadFactor) {
        new_capacity = jimi::detail::round_up_pow2(new_capacity);
        assert(new_capacity > 0);
        assert((new_capacity & (new_capacity - 1)) == 0);
        list_type ** new_table = new (std::nothrow) list_type *[new_capacity];
        if (likely(new_table != nullptr)) {
            // Initialize the table data.
            memset((void *)new_table, 0, sizeof(list_type *) * new_capacity);
            // Setting status
            this->table_ = new_table;
            this->size_ = 0;
            this->mask_ = new_capacity - 1;
            this->capacity_ = new_capacity;           
            assert(loadFactor > 0.0);
            this->threshold_ = (size_type)(new_capacity * std::fabs(loadFactor));
            this->loadFactor_ = loadFactor;
        }
    }

    void destroy() {
        // Clear all data, and free the table.
        if (likely(this->table_ != nullptr)) {
            for (size_type i = 0; i < this->capacity_; ++i) {
                list_type * list = (list_type *)this->table_[i];
                if (likely(list != nullptr)) {
                    //delete list;
                    jstd::nothrow_deleter::destroy(list);
#ifndef NDEBUG
                    this->table_[i] = nullptr;
#endif
                }
            }
            //delete[] this->table_;
            jstd::nothrow_deleter::destroy(this->table_);
            this->table_ = nullptr;
        }
#ifndef NDEBUG
        // Setting status
        this->size_ = 0;
        this->mask_ = 0;
        this->capacity_ = 0;
        this->threshold_ = 0;
#endif
    }

#if USE_JSTD_HASH_MAP
    inline size_type calc_capacity(size_type new_capacity) {
        // If new_capacity is less than half of the current hash table size,
        // then double the hash table size.
        new_capacity = (new_capacity > (this->size_ * 2)) ? new_capacity : (this->size_ * 2);
        // The minimum bucket is kDefaultInitialCapacity = 64.
        new_capacity = (new_capacity >= kDefaultInitialCapacity) ? new_capacity : kDefaultInitialCapacity;
        // The maximum bucket is kMaximumCapacity = 1 << 30.
        new_capacity = (new_capacity <= kMaximumCapacity) ? new_capacity : kMaximumCapacity;
        // Round up the new_capacity to power 2.
        new_capacity = jimi::detail::round_up_pow2(new_capacity);
        return new_capacity;
    }

    inline size_type calc_capacity_fast(size_type new_capacity) {
        // If new_capacity is less than half of the current hash table size,
        // then double the hash table size.
        new_capacity = (new_capacity > (this->size_ * 2)) ? new_capacity : (this->size_ * 2);
        // The maximum bucket is kMaximumCapacity = 1 << 30.
        new_capacity = (new_capacity <= kMaximumCapacity) ? new_capacity : kMaximumCapacity;
        // Round up the new_capacity to power 2.
        new_capacity = jimi::detail::round_up_pow2(new_capacity);
        return new_capacity;
    }

    static inline
    hash_type hash(const key_type & key) {
        return jstd::hash_helper<key_type, hash_type, HashFunc>::getHashCode(key);
    }

    static inline
    size_type index_of(hash_type hash, size_type mask) {
#if 0
        size_type index = ((size_type)hash % (mask + 1));
#else
        size_type index = ((size_type)hash & mask);
#endif
        return index;
    }

    void reserve_internal(size_type new_capacity) {
        assert(new_capacity > 0);
        assert((new_capacity & (new_capacity - 1)) == 0);
        if (likely(new_capacity > this->capacity_)) {
            list_type ** new_table = new (std::nothrow) list_type *[new_capacity];
            if (new_table != nullptr) {
                // Initialize the table data.
                memset((void *)new_table, 0, sizeof(list_type *) * new_capacity);
                if (likely(this->table_ != nullptr)) {
                    //delete[] this->table_;
                    jstd::nothrow_deleter::destroy(this->table_);
                }
                // Setting status
                this->table_ = new_table;
                this->size_ = 0;
                this->mask_ = new_capacity - 1;
                this->capacity_ = new_capacity;
                assert(this->loadFactor_ > 0.0);
                this->threshold_ = (size_type)(new_capacity * std::fabs(this->loadFactor_));
            }
        }
    }

    //JM_FORCEINLINE_DECLARE(void)
    void reinsert_list(list_type ** new_table, size_type new_mask,
                       list_type * old_list) {
        assert(new_table != nullptr);
        assert(old_list != nullptr);
        assert(new_mask > 0);

        entry_type * old_entry = old_list->head();
        while (likely(old_entry != nullptr)) {
            hash_type hash = old_entry->hash;
            size_type index = this_type::index_of(hash, new_mask);

            list_type * list = new_table[index];
            if (likely(list == nullptr)) {
                // Create the new list and push the old entry to front of new list.
                list_type * new_list = new (std::nothrow) list_type(old_entry);
                if (likely(new_list != nullptr)) {
                    assert(new_table[index] == nullptr);
                    new_table[index] = new_list;
                    ++(this->size_);

                    // Save the value of old_entry->next.
                    entry_type * next_entry = old_entry->next;
                    // Modify the value of old_entry->next.
                    old_entry->next = nullptr;

                    // Scan next entry
                    old_entry = next_entry;
                }
                else {
                    // Error: out of memory
                    //printf("Error: out of memory !\n");
                }
            }
            else {
                // Push the old entry to front of new list.
                assert(list != nullptr);
                // Save the value of old_entry->next.
                entry_type * next_entry = old_entry->next;

                // Push the old entry to front of new list.
                entry_type * head = list->head();
                if (likely(head != nullptr)) {
                    list->push_front_fast(old_entry);
                    ++(this->size_);

                    // Scan next entry
                    old_entry = next_entry;
                }
                else {
                    assert(list->head() == nullptr);
                    list->push_first(old_entry);
                    ++(this->size_);

                    // Scan next entry
                    old_entry = next_entry;
                }
            }
        }
    }

    template <bool force_shrink = false>
    void rehash_internal(size_type new_capacity) {
        assert(new_capacity > 0);
        assert((new_capacity & (new_capacity - 1)) == 0);
        assert(new_capacity >= this->size_ * 2);
        if (likely((force_shrink == false && new_capacity > this->capacity_) ||
                   (force_shrink == true && new_capacity != this->capacity_))) {
            list_type ** new_table = new (std::nothrow) list_type *[new_capacity];
            if (likely(new_table != nullptr)) {
                // Initialize the new table data.
                memset((void *)new_table, 0, sizeof(list_type *) * new_capacity);

                // Recalculate the bucket of all keys.
                if (likely(this->table_ != nullptr)) {
                    size_type old_size = this->size_;
                    this->size_ = 0;
                    size_type new_mask = new_capacity - 1;

                    for (size_type i = 0; i < this->capacity_; ++i) {
                        list_type * old_list = this->table_[i];
                        if (likely(old_list != nullptr)) {
                            // Insert the old buckets to the new buckets in the new table.
                            this->reinsert_list(new_table, new_mask, old_list);
                            // Set the old_list->head to nullptr.
                            old_list->reset();
                            // Destory the old list.
                            //delete old_list;
                            jstd::nothrow_deleter::destroy(old_list);
#ifndef NDEBUG
                            // Set to nullptr.
                            this->table_[i] = nullptr;
#endif
                        }
                    }
                    assert(this->size_ == old_size);

                    // Free old table data.
                    //delete[] this->table_;
                    jstd::nothrow_deleter::destroy(this->table_);
                }
                // Setting status
                this->table_ = new_table;
                this->mask_ = new_capacity - 1;
                this->capacity_ = new_capacity;
                assert(this->loadFactor_ > 0.0);
                this->threshold_ = (size_type)(new_capacity * std::fabs(this->loadFactor_));
            }
        }
    }

    void resize_internal(size_type new_capacity) {
        this->rehash_internal<false>(new_capacity);
    }

public:
    void clear() {
        // Clear the data only, don't free the table.
        if (likely(this->table_ != nullptr)) {
            for (size_type i = 0; i < this->capacity_; ++i) {
                list_type * list = (list_type *)this->table_[i];
                if (likely(list != nullptr)) {
                    list->clear();
                }
            }
        }
        // Setting status
        this->size_ = 0;
    }

    void reserve(size_type new_capacity) {
        // Recalculate the size of new_capacity.
        new_capacity = this->calc_capacity(new_capacity);
        this->reserve_internal(new_capacity);
    }

    void rehash(size_type new_capacity) {
        // Recalculate the size of new_capacity.
        new_capacity = this->calc_capacity(new_capacity);
        this->rehash_internal<false>(new_capacity);
    }

    void resize(size_type new_capacity) {
        this->rehash(new_capacity);
    }

    void shrink_to_fit(size_type new_capacity) {
        // Recalculate the size of new_capacity.
        new_capacity = this->calc_capacity_fast(new_capacity);
        this->rehash_internal<true>(new_capacity);
    }

    iterator find(const key_type & key) {
        if (likely(this->table_ != nullptr)) {
            hash_type hash = this_type::hash(key);
            size_type index = this_type::index_of(hash, this->mask_);

            list_type * list = this->table_[index];
            if (likely(list != nullptr)) {
                entry_type * entry = list->head();
                while (likely(entry != nullptr)) {
                    // Found a entry, next to check the hash value.
                    if (likely(entry->hash != hash)) {
                        // Scan next entry
                        entry = entry->next;
                    }
                    else {
                        // If hash value is equal, then compare the key sizes and the strings.
                        if (likely(key.size() == entry->pair.first.size())) {
    #if USE_SSE42_STRING_COMPARE
                            if (likely(StrUtils::is_equals_fast(key, entry->pair.first))) {
                                return (iterator)&this->table_[index];
                            }
    #else
                            if (likely(strcmp(key.c_str(), entry->pair.first.c_str()) == 0)) {
                                return (iterator)&this->table_[index];
                            }
    #endif
                        }
                        // Scan next entry
                        entry = entry->next;
                    }
                }
            }
        }

        // Not found
        return this->end();
    }

    iterator find_internal(const key_type & key, hash_type & hash, size_type & index) {
        hash = this_type::hash(key);
        index = this_type::index_of(hash, this->mask_);

        assert(this->table_ != nullptr);
        list_type * list = this->table_[index];
        if (likely(list != nullptr)) {
            entry_type * entry = list->head();
            while (likely(entry != nullptr)) {
                // Found a entry, next to check the hash value.
                if (likely(entry->hash != hash)) {
                    // Scan next entry
                    entry = entry->next;
                }
                else {
                    // If hash value is equal, then compare the key sizes and the strings.
                    if (likely(key.size() == entry->pair.first.size())) {
#if USE_SSE42_STRING_COMPARE
                        if (likely(StrUtils::is_equals_fast(key, entry->pair.first))) {
                            return (iterator)entry;
                        }
#else
                        if (likely(strcmp(key.c_str(), entry->pair.first.c_str()) == 0)) {
                            return (iterator)entry;
                        }
#endif
                    }
                    // Scan next entry
                    entry = entry->next;
                }
            }
        }

        // Not found
        return this->end();
    }

    iterator find_before(const key_type & key, entry_type *& before_out, size_type & index) {
        hash_type hash = this_type::hash(key);
        index = this_type::index_of(hash, this->mask_);

        assert(this->table_ != nullptr);
        list_type * list = this->table_[index];
        if (likely(list != nullptr)) {
            entry_type * before = nullptr;
            entry_type * entry = list->head();
            while (likely(entry != nullptr)) {
                // Found entry, next to check the hash value.
                if (likely(entry->hash != hash)) {
                    // Scan next entry
                    before = entry;
                    entry = entry->next;
                }
                else {
                    // If hash value is equal, then compare the key sizes and the strings.
                    if (likely(key.size() == entry->pair.first.size())) {
#if USE_SSE42_STRING_COMPARE
                        if (likely(StrUtils::is_equals_fast(key, entry->pair.first))) {
                            before_out = before;
                            return (iterator)entry;
                        }
#else
                        if (likely(strcmp(key.c_str(), entry->pair.first.c_str()) == 0)) {
                            before_out = before;
                            return (iterator)entry;
                        }
#endif
                    }
                    // Scan next entry
                    before = entry;
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
                    index = this_type::index_of(hash, this->mask_);
                }

                entry_type * new_entry = new (std::nothrow) entry_type(hash, key, value);
                if (likely(new_entry != nullptr)) {
                    list_type * list = this->table_[index];
                    if (likely(list == nullptr)) {
                        // Create new list and push the new entry to front of new list.
                        list_type * new_list = new (std::nothrow) list_type(new_entry);
                        if (likely(new_list != nullptr)) {
                            assert(this->table_[index] == nullptr);
                            this->table_[index] = new_list;
                            ++(this->size_);
                        }
                        else {
                            // Error: out of memory
                        }
                    }
                    else {
                        // Push the new entry to front of new list.
                        assert(list != nullptr);
                        entry_type * head = list->head();
                        if (likely(head != nullptr)) {
                            assert(list->head() != nullptr);
                            list->push_front_fast(new_entry);
                            ++(this->size_);
                        }
                        else {
                            assert(list->head() == nullptr);
                            list->push_first_fast(new_entry);
                            ++(this->size_);
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
                    index = this_type::index_of(hash, this->mask_);
                }

                entry_type * new_entry = new (std::nothrow) entry_type(hash,
                                                        std::forward<key_type>(key),
                                                        std::forward<value_type>(value));
                if (likely(new_entry != nullptr)) {
                    list_type * list = this->table_[index];
                    if (likely(list == nullptr)) {
                        // Create new list and push the new entry to front of new list.
                        list_type * new_list = new (std::nothrow) list_type(new_entry);
                        if (likely(new_list != nullptr)) {
                            assert(this->table_[index] == nullptr);
                            this->table_[index] = new_list;
                            ++(this->size_);
                        }
                        else {
                            // Error: out of memory
                        }
                    }
                    else {
                        // Push the new entry to front of new list.
                        assert(list != nullptr);
                        entry_type * head = list->head();
                        if (likely(head != nullptr)) {
                            assert(list->head() != nullptr);
                            list->push_front_fast(new_entry);
                            ++(this->size_);
                        }
                        else {
                            assert(list->head() == nullptr);
                            list->push_first_fast(new_entry);
                            ++(this->size_);
                        }
                    }
                }
            }
            else {
                // Update the existed key's value.
                assert(iter != nullptr);
                //iter->pair.second.swap(value);
                std::swap(iter->pair.second, value);
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

    void emplace(const key_type & key, const value_type & value) {
        this->emplace(std::make_pair(key, value));
    }

    void emplace(key_type && key, value_type && value) {
        this->emplace(std::make_pair(std::forward<key_type>(key),
                                     std::forward<value_type>(value)));
    }

    void erase(const key_type & key) {
        if (likely(this->table_ != nullptr)) {
            entry_type * before;
            size_type index;
            iterator iter = this->find_before(key, before, index);
            if (likely(iter != this->end())) {
                entry_type * entry = (entry_type *)iter;
                assert(entry != nullptr);
                assert(this->size_ > 0);

                list_type * list = this->table_[index];
                assert(list != nullptr);
                assert((before != nullptr && entry != list->head()) ||
                       (before == nullptr && entry == list->head()));
                list->erase_fast(before);

                assert(this->size_ > 0);
                --(this->size_);
            }
            else {
                // Not found the key
            }
        }
    }

    void erase(key_type && key) {
        if (likely(this->table_ != nullptr)) {
            entry_type * before;
            size_type index;
            iterator iter = this->find_before(std::forward<key_type>(key), before, index);
            if (likely(iter != this->end())) {
                entry_type * entry = (entry_type *)iter;
                assert(entry != nullptr);
                assert(this->size_ > 0);

                list_type * list = this->table_[index];
                assert(list != nullptr);
                assert((before != nullptr && entry != list->head()) ||
                       (before == nullptr && entry == list->head()));
                list->erase_fast(before);

                assert(this->size_ > 0);
                --(this->size_);
            }
            else {
                // Not found the key
            }
        }
    }
#endif // USE_JSTD_HASH_MAP

    static const char * name() {
        switch (HashFunc) {
            case HashFunc_CRC32C:
                return "jstd::hash_map<K, V> (CRC32c)";
            case HashFunc_Time31:
                return "jstd::hash_map<K, V> (Time31)";
            case HashFunc_Time31Std:
                return "jstd::hash_map<K, V> (Time31Std)";
            case HashFunc_SHA1_MSG2:
                return "jstd::hash_map<K, V> (SHA1_Msg2)";
            case HashFunc_SHA1:
                return "jstd::hash_map<K, V> (SHA1)";
            default:
                return "Unknown class name";
        }
    }
};

template <typename Key, typename Value, std::size_t HashFunc>
const typename basic_hash_map<Key, Value, HashFunc>::float_type
basic_hash_map<Key, Value, HashFunc>::kDefaultLoadFactor = 0.75;

#if SUPPORT_SSE42_CRC32C
template <typename Key, typename Value>
using hash_map = basic_hash_map<Key, Value, HashFunc_CRC32C>;
#endif

template <typename Key, typename Value>
using hash_map_time31 = basic_hash_map<Key, Value, HashFunc_Time31>;

template <typename Key, typename Value>
using hash_map_time31_std = basic_hash_map<Key, Value, HashFunc_Time31Std>;

#if SUPPORT_SMID_SHA
template <typename Key, typename Value>
using hash_map_sha1_msg2 = basic_hash_map<Key, Value, HashFunc_SHA1_MSG2>;
#endif

#if SUPPORT_SMID_SHA
template <typename Key, typename Value>
using hash_map_sha1 = basic_hash_map<Key, Value, HashFunc_SHA1>;
#endif

} // namespace jstd

#endif // JSTD_HASH_MAP_H
