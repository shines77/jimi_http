
#ifndef JSTD_DICTIONARY_EX_H
#define JSTD_DICTIONARY_EX_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "jimi/basic/stddef.h"
#include "jimi/basic/stdint.h"
#include "jimi/basic/stdsize.h"

#include <cstddef>
#include <memory>
#include <type_traits>

#include "jimi/jstd/dictionary_traits.h"
#include "jimi/support/Power2.h"

#define USE_JSTD_DICTIONARY_EX      1

#define SUPPORT_DICTIONARY_VERSION  0

namespace jstd {

template <typename Key, typename Value, std::size_t HashFunc = HashFunc_Default,
          typename Traits = default_dictionary_traits<Key, Value, HashFunc>>
class basic_dictionary_ex {
public:
    typedef Key                             key_type;
    typedef Value                           value_type;
    typedef Traits                          traits_type;
    typedef std::pair<Key, Value>           pair_type;
    typedef typename Traits::size_type      size_type;
    typedef typename Traits::hash_type      hash_type;
    typedef typename Traits::index_type     index_type;
    typedef basic_dictionary_ex<Key, Value, HashFunc, Traits>
                                            this_type;

    struct hash_entry {
        hash_entry * next;
        hash_type    hash;
        pair_type    pair;

        hash_entry() : next(nullptr), hash(0) {}
        hash_entry(hash_type hash_code) : next(nullptr), hash(hash_code) {}

        hash_entry(hash_type hash_code, const key_type & key,
              const value_type & value, hash_entry * next_entry = nullptr)
            : next(next_entry), hash(hash_code), pair(key, value) {}
        hash_entry(hash_type hash_code, key_type && key,
              value_type && value, hash_entry * next_entry = nullptr)
            : next(next_entry), hash(hash_code),
              pair(std::forward<key_type>(key), std::forward<value_type>(value)) {}

        hash_entry(const key_type & key, const value_type & value)
            : next(nullptr), hash(0), pair(key, value) {}
        hash_entry(key_type && key, value_type && value)
            : next(nullptr), hash(0),
              pair(std::forward<key_type>(key), std::forward<value_type>(value)) {}

        ~hash_entry() {
#ifndef NDEBUG
            this->next = nullptr;
#endif
        }
    };

    typedef hash_entry          entry_type;
    typedef entry_type *        iterator;
    typedef const entry_type *  const_iterator;

    class free_list {
    private:
        entry_type * head_;
        size_type    size_;

    public:
        free_list() : head_(nullptr), size_(0) {}
        free_list(hash_entry * head) : head_(head), size_(0) {}
        ~free_list() {
#ifndef NDEBUG
            this->clear();
#endif
        }

        entry_type * begin() const { return this->head_; }
        entry_type * end() const { return nullptr; }

        entry_type * head() const { return this->head_; }
        size_type size() const { return this->size_; }

        void set_head(entry_type * new_entry) {
            this->head_ = new_entry;
        }
        void set_size(size_type new_size) {
            this->size_ = new_size;
        }

        bool is_valid() const { return (this->head_ != nullptr); }
        bool is_empty() const { return (this->size_ == 0); }

        void clear() {
            this->head_ = nullptr;
            this->size_ = 0;
        }

        void reset(hash_entry * head) {
            this->head_ = head;
            this->size_ = 0;
        }

        void increase() {
            ++(this->size_);
        }

        void decrease() {
            --(this->size_);
        }

        void push_first(entry_type * entry) {
            assert(entry != nullptr);
            assert(entry->next == nullptr);
            this->head_ = entry;
            ++(this->size_);
        }

        void push_front(entry_type * entry) {
            assert(entry != nullptr);
            entry->next = this->head_;
            this->head_ = entry;
            ++(this->size_);
        }

        entry_type * pop_front() {
            entry_type * entry = this->head_;
            assert(entry != nullptr);
            this->head_ = entry->next;
            assert(this->size_ > 0);
            --(this->size_);
            return entry;
        }

        void swap(free_list & right) {
            if (&right != this) {
                entry_type * save_head = this->head_;
                size_type save_size = this->size_;
                this->head_ = right.head_;
                this->size_ = right.size_;
                right.head_ = save_head;
                right.size_ = save_size;
            }
        }
    };

    inline void swap(free_list & lhs, free_list & rhs) {
        lhs.swap(rhs);
    }

    typedef free_list list_type;

private:
    entry_type **   buckets_;
    entry_type *    entries_;
    size_type       size_;
    size_type       count_;
    size_type       mask_;
    size_type       capacity_;
    list_type       freelist_;
    size_type       threshold_;
    float           loadFactor_;
#if SUPPORT_DICTIONARY_VERSION
    size_type       version_;
#endif
    traits_type     traits_;

    // Default initial capacity is 64.
    static const size_type kDefaultInitialCapacity = 64;
    // Maximum capacity is 1 << 30.
    static const size_type kMaximumCapacity = 1U << 30;
    // Default load factor is: 0.75
    static const float kDefaultLoadFactor;
    // The threshold of treeify to red-black tree.
    static const size_type kTreeifyThreshold = 8;
    // The invalid hash value.
    static const hash_type kInvalidHash = static_cast<hash_type>(-1);

public:
    basic_dictionary_ex(size_type initialCapacity = kDefaultInitialCapacity,
                     float loadFactor = kDefaultLoadFactor)
        : buckets_(nullptr), entries_(nullptr), size_(0), count_(0), mask_(0),
          capacity_(0), threshold_(0), loadFactor_(kDefaultLoadFactor)
#if SUPPORT_DICTIONARY_VERSION
          , version_(1)
#endif
    {
        this->initialize(initialCapacity, loadFactor);
    }

    ~basic_dictionary_ex() {
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

    size_type version() const {
#if SUPPORT_DICTIONARY_VERSION
        return this->version_;
#else
        return 0;
#endif
    }

    void clear() {
        if (likely(this->buckets_ != nullptr)) {
            // Initialize the buckets's data.
            memset((void *)this->buckets_, 0, sizeof(entry_type *) * this->capacity_);
        }
        // Setting status
        this->size_ = 0;
        this->count_ = 0;
        this->freelist_.clear();
    }

private:
    // Linked the entries to the free list.
    void fill_freelist(list_type & freelist, entry_type * entries, size_type capacity) {
        assert(entries != nullptr);
        assert(capacity > 0);
        entry_type * entry = entries;
        for (size_type i = 0; i < capacity; ++i) {
            entry_type * next_entry = entry + 1;
            entry->next = next_entry;
            entry = next_entry;
        }
        freelist.set_head(entries);
        freelist.set_size(capacity);
    }

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
                // Linked all new entries to the free list.
                //fill_freelist(this->freelist_, new_entries, new_capacity);

                // Initialize status
                this->entries_ = new_entries;
                this->size_ = 0;
                this->count_ = 0;
                this->mask_ = new_capacity - 1;
                this->capacity_ = new_capacity;

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
        this->size_ = 0;
        this->count_ = 0;
        this->mask_ = 0;
        this->capacity_ = 0;
        this->threshold_ = 0;
#endif
    }

    inline size_type calc_capacity(size_type new_capacity) {
        // The minimum bucket is kDefaultInitialCapacity = 64.
        new_capacity = (new_capacity >= kDefaultInitialCapacity) ? new_capacity : kDefaultInitialCapacity;
        // The maximum bucket is kMaximumCapacity = 1 << 30.
        new_capacity = (new_capacity <= kMaximumCapacity) ? new_capacity : kMaximumCapacity;
        // Round up the new_capacity to power 2.
        new_capacity = jimi::detail::round_up_pow2(new_capacity);
        return new_capacity;
    }

    inline size_type calc_shrink_capacity(size_type new_capacity) {
        // The maximum bucket is kMaximumCapacity = 1 << 30.
        new_capacity = (new_capacity <= kMaximumCapacity) ? new_capacity : kMaximumCapacity;
        // Round up the new_capacity to power 2.
        new_capacity = jimi::detail::round_up_pow2(new_capacity);
        return new_capacity;
    }

    void updateVersion() {
#if SUPPORT_DICTIONARY_VERSION
        ++(this->version_);
#endif
    }

    void reinsert_list(entry_type ** new_buckets, free_list * new_freelist,
                       size_type new_mask, entry_type * old_entry) {
        assert(new_buckets != nullptr);
        assert(new_freelist != nullptr);
        assert(old_entry != nullptr);
        assert(new_mask > 0);

        do {
            hash_type hash = old_entry->hash;
            size_type index = this->traits_.index_of(hash, new_mask);

            // Save the value of old_entry->next.
            entry_type * next_entry = old_entry->next;

            // Push the old entry to front of new list.
            old_entry->next = new_buckets[index];
            new_buckets[index] = old_entry;
            ++(this->size_);

            // Scan next entry
            old_entry = next_entry;
        } while (likely(old_entry != nullptr));
    }

    void rehash_internal(size_type new_capacity) {
        assert(new_capacity > 0);
        assert((new_capacity & (new_capacity - 1)) == 0);
        if (likely(new_capacity > this->capacity_)) {
            // The the array of bucket's first entry.
            entry_type ** new_buckets = new entry_type *[new_capacity];
            if (likely(new_buckets != nullptr)) {
                // Initialize the buckets's data.
                memset((void *)new_buckets, 0, sizeof(entry_type *) * new_capacity);

                // The the array of entries.
                entry_type * new_entries = new entry_type[new_capacity];
                if (likely(new_entries != nullptr)) {
                    // Linked all new entries to the new free list.
                    free_list new_freelist;
                    //fill_freelist(new_freelist, new_entries, new_capacity);

                    // Recalculate the bucket of all keys.
                    if (likely(this->buckets_ != nullptr)) {
                        size_type old_size = this->size_;
                        this->size_ = 0;

                        entry_type ** old_buckets = this->buckets_;
                        size_type new_mask = new_capacity - 1;

                        for (size_type i = 0; i < this->capacity_; ++i) {
                            assert(old_buckets != nullptr);
                            entry_type * old_entry = *old_buckets;
                            if (likely(old_entry == nullptr)) {
                                old_buckets++;
                            }
                            else {
                                // Insert the old buckets to the new buckets in the new table.
                                this->reinsert_list(new_buckets, &new_freelist, new_mask, old_entry);
    #ifndef NDEBUG
                                // Set the old_list.head to nullptr.
                                *old_buckets = nullptr;
    #endif
                                old_buckets++;
                            }
                        }
                        assert(this->size_ == old_size);

                        // Free old buckets data.
                        delete[] this->buckets_;
                    }

                    // Setting status
                    this->buckets_ = new_buckets;
                    this->entries_ = new_entries;
                    this->count_ = 0;
                    this->mask_ = new_capacity - 1;
                    this->capacity_ = new_capacity;

                    this->freelist_.swap(new_freelist);

                    assert(this->loadFactor_ > 0.0f);
                    this->threshold_ = (size_type)(new_capacity * fabsf(this->loadFactor_));

                    this->updateVersion();
                }
            }
        }
    }

    void shrink_internal(size_type new_capacity) {
        assert(new_capacity > 0);
        assert((new_capacity & (new_capacity - 1)) == 0);
        if (likely(this->capacity_ != new_capacity)) {
            // The the array of bucket's first entry.
            entry_type ** new_buckets = new entry_type *[new_capacity];
            if (likely(new_buckets != nullptr)) {
                // Initialize the buckets's data.
                memset((void *)new_buckets, 0, sizeof(entry_type *) * new_capacity);

                // The the array of entries.
                entry_type * new_entries = new entry_type[new_capacity];
                if (likely(new_entries != nullptr)) {
                    // Linked all new entries to the new free list.
                    free_list new_freelist;
                    //fill_freelist(new_freelist, new_entries, new_capacity);

                    // Recalculate the bucket of all keys.
                    if (likely(this->buckets_ != nullptr)) {
                        size_type old_size = this->size_;
                        this->size_ = 0;

                        entry_type ** old_buckets = this->buckets_;
                        size_type new_mask = new_capacity - 1;

                        for (size_type i = 0; i < this->capacity_; ++i) {
                            assert(old_buckets != nullptr);
                            entry_type * old_entry = *old_buckets;
                            if (likely(old_entry == nullptr)) {
                                old_buckets++;
                            }
                            else {
                                // Insert the old buckets to the new buckets in the new table.
                                this->reinsert_list(new_buckets, &new_freelist, new_mask, old_entry);
    #ifndef NDEBUG
                                // Set the old_list.head to nullptr.
                                *old_buckets = nullptr;
    #endif
                                old_buckets++;
                            }
                        }
                        assert(this->size_ == old_size);

                        // Free old buckets data.
                        delete[] this->buckets_;
                    }

                    // Setting status
                    this->buckets_ = new_buckets;
                    this->entries_ = new_entries;
                    this->count_ = 0;
                    this->mask_ = new_capacity - 1;
                    this->capacity_ = new_capacity;

                    assert(this->loadFactor_ > 0.0f);
                    this->threshold_ = (size_type)(new_capacity * fabsf(this->loadFactor_));

                    this->updateVersion();
                }
            }
        }
    }

    void resize_internal(size_type new_capacity) {
        assert(new_capacity > 0);
        assert((new_capacity & (new_capacity - 1)) == 0);
        this->rehash_internal(new_capacity);
    }

public:
    void dump() {
        printf("jstd::basic_dictionary_ex<K, V>::dump()\n\n");
    }

    void reserve(size_type new_capacity) {
        // Recalculate the size of new_capacity.
        new_capacity = this->calc_capacity(new_capacity);
        this->rehash_internal(new_capacity);
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
        new_capacity = this->calc_shrink_capacity(new_capacity);
        this->shrink_internal(new_capacity);
    }

    iterator find(const key_type & key) {
        if (likely(this->buckets() != nullptr)) {
            hash_type hash = this->traits_.hash_code(key);
            index_type index = this->traits_.index_of(hash, this->mask_);

            assert(this->entries() != nullptr);
            entry_type * entry = this->buckets_[index];
            while (likely(entry != nullptr)) {
                // Found a entry, next to check the hash value.
                if (likely(entry->hash != hash)) {
                    // Scan next entry
                    entry = entry->next;
                }
                else {
                    // If hash value is equal, then compare the key sizes and the strings.
                    if (likely(this->traits_.key_is_equals(key, entry->pair.first))) {
                        return (iterator)entry;
                    }
                }
            }

            // Not found
            return this->unsafe_end();
        }

        // Not found
        return nullptr;
    }

    inline iterator find_internal(const key_type & key, hash_type & hash, index_type & index) {
        hash = this->traits_.hash_code(key);
        index = this->traits_.index_of(hash, this->mask_);

        assert(this->buckets() != nullptr);
        assert(this->entries() != nullptr);
        entry_type * entry = this->buckets_[index];
        while (likely(entry != nullptr)) {
            // Found a entry, next to check the hash value.
            if (likely(entry->hash != hash)) {
                // Scan next entry
                entry = entry->next;
            }
            else {
                // If hash value is equal, then compare the key sizes and the strings.
                if (likely(this->traits_.key_is_equals(key, entry->pair.first))) {
                    return (iterator)entry;
                }
            }
        }

        // Not found
        return this->unsafe_end();
    }

    inline iterator find_before(const key_type & key, entry_type *& before_out, size_type & index) {
        hash_type hash = this->traits_.hash_code(key);
        index = this->traits_.index_of(hash, this->mask_);

        assert(this->buckets() != nullptr);
        assert(this->entries() != nullptr);
        entry_type * before = nullptr;
        entry_type * entry = this->buckets_[index];
        while (likely(entry != nullptr)) {
            // Found entry, next to check the hash value.
            if (likely(entry->hash != hash)) {
                // Scan next entry
                before = entry;
                entry = entry->next;
            }
            else {
                // If hash value is equal, then compare the key sizes and the strings.
                if (likely(this->traits_.key_is_equals(key, entry->pair.first))) {
                    before_out = before;
                    return (iterator)entry;
                }
            }
        }

        // Not found
        return this->unsafe_end();
    }

    bool contains(const key_type & key) {
        iterator iter = this->find(key);
        return (iter != this->end());
    }

    void insert(const key_type & key, const value_type & value) {
        if (likely(this->buckets_ != nullptr)) {
            hash_type hash;
            index_type index;
            iterator iter = this->find_internal(key, hash, index);
            if (likely(iter == this->unsafe_end())) {
                // Insert the new key.
                entry_type * new_entry;
                if (likely(this->freelist_.is_empty())) {
                    if (likely((this->count_ >= this->capacity_ ||
                                this->size_ >= this->capacity_))) {
                        // Resize the buckets
                        this->resize_internal(this->capacity_ * 2);
                        // Recalculate the index.
                        index = this->traits_.index_of(hash, this->mask_);
                    }

                    // Get a unused entry.
                    new_entry = &this->entries_[this->count_];
                    assert(new_entry != nullptr);
                    ++(this->count_);
                }
                else {
                    // Pop a free entry from freelist.
                    new_entry = this->freelist_.pop_front();
                    assert(new_entry != nullptr);
                }

                new_entry->next = this->buckets_[index];
                new_entry->hash = hash;
                new_entry->pair.first = key;
                new_entry->pair.second = value;

                this->buckets_[index] = new_entry;
                ++(this->size_);

                this->updateVersion();
            }
            else {
                // Update the existed key's value.
                assert(iter != nullptr);
                iter->pair.second = value;

                this->updateVersion();
            }
        }
    }

    void insert(key_type && key, value_type && value) {
        if (likely(this->buckets_ != nullptr)) {
            hash_type hash;
            index_type index;
            iterator iter = this->find_internal(std::forward<key_type>(key), hash, index);
            if (likely(iter == this->unsafe_end())) {
                // Insert the new key.
                entry_type * new_entry;
                if (likely(this->freelist_.is_empty())) {
                    if (likely((this->count_ >= this->capacity_ ||
                                this->size_ >= this->capacity_))) {
                        // Resize the buckets
                        this->resize_internal(this->capacity_ * 2);
                        // Recalculate the index.
                        index = this->traits_.index_of(hash, this->mask_);
                    }

                    // Get a unused entry.
                    new_entry = &this->entries_[this->count_];
                    assert(new_entry != nullptr);
                    ++(this->count_);
                }
                else {
                    // Pop a free entry from freelist.
                    new_entry = this->freelist_.pop_front();
                    assert(new_entry != nullptr);
                }

                new_entry->next = this->buckets_[index];
                new_entry->hash = hash;
                new_entry->pair.first.swap(key);
                new_entry->pair.second.swap(value);

                this->buckets_[index] = new_entry;
                ++(this->size_);

                this->updateVersion();
            }
            else {
                // Update the existed key's value.
                assert(iter != nullptr);
                iter->pair.second.swap(value);

                this->updateVersion();
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

#if 0
    bool erase(const key_type & key) {
        if (likely(this->buckets_ != nullptr)) {
            assert(this->entries() != nullptr);
            entry_type * before;
            size_type index;
            iterator iter = this->find_before(key, before, index);
            if (likely(iter != this->unsafe_end())) {
                entry_type * entry = (entry_type *)iter;
                assert(entry != nullptr);

                if (likely(before != nullptr))
                    before->next = entry->next;
                else
                    this->buckets_[index] = entry->next;

                entry->next = this->freelist_.head();
                entry->hash = kInvalidHash;
                entry->pair.first.clear();
                entry->pair.second.clear();

                this->freelist_.set_head(entry);
                this->freelist_.increase();

                assert(this->size_ > 0);
                --(this->size_);

                this->updateVersion();

                // Has found the key.
                return true;
            }
        }

        // Not found the key.
        return false;
    }

    bool erase(key_type && key) {
        if (likely(this->buckets_ != nullptr)) {
            assert(this->entries() != nullptr);
            entry_type * before;
            size_type index;
            iterator iter = this->find_before(std::forward<key_type>(key), before, index);
            if (likely(iter != this->unsafe_end())) {
                entry_type * entry = (entry_type *)iter;
                assert(entry != nullptr);

                if (likely(before != nullptr))
                    before->next = entry->next;
                else
                    this->buckets_[index] = entry->next;

                entry->next = this->freelist_.head();
                entry->hash = kInvalidHash;
                entry->pair.first.clear();
                entry->pair.second.clear();

                this->freelist_.set_head(entry);
                this->freelist_.increase();

                assert(this->size_ > 0);
                --(this->size_);

                this->updateVersion();

                // Has found the key.
                return true;
            }
        }

        // Not found the key.
        return false;
    }
#else
    bool erase(const key_type & key) {
        if (likely(this->buckets_ != nullptr)) {
            hash_type hash = this->traits_.hash_code(key);
            size_type index = this->traits_.index_of(hash, this->mask_);

            assert(this->buckets() != nullptr);
            assert(this->entries() != nullptr);
            entry_type * before = nullptr;
            entry_type * entry = this->buckets_[index];
            while (likely(entry != nullptr)) {
                // Found a entry, next to check the hash value.
                if (likely(entry->hash != hash)) {
                    // Scan next entry
                    before = entry;
                    entry = entry->next;
                }
                else {
                    // If hash value is equal, then compare the key sizes and the strings.
                    if (likely(this->traits_.key_is_equals(key, entry->pair.first))) {
                        if (likely(before != nullptr))
                            before->next = entry->next;
                        else
                            this->buckets_[index] = entry->next;

                        entry->next = this->freelist_.head();
                        entry->hash = kInvalidHash;
#ifdef _MSC_VER
                        entry->pair.first.clear();
                        entry->pair.second.clear();
#else
                        entry->pair.first = std::string("");
                        entry->pair.second = std::string("");
#endif
                        this->freelist_.set_head(entry);
                        this->freelist_.increase();

                        assert(this->size_ > 0);
                        --(this->size_);

                        this->updateVersion();

                        // Has found the key.
                        return true;
                    }
                }
            }
        }

        // Not found the key.
        return false;
    }

    bool erase(key_type && key) {
        if (likely(this->buckets_ != nullptr)) {
            hash_type hash = this->traits_.hash_code(std::forward<key_type>(key));
            size_type index = this->traits_.index_of(hash, this->mask_);

            assert(this->buckets() != nullptr);
            assert(this->entries() != nullptr);
            entry_type * before = nullptr;
            entry_type * entry = this->buckets_[index];
            while (likely(entry != nullptr)) {
                // Found a entry, next to check the hash value.
                if (likely(entry->hash != hash)) {
                    // Scan next entry
                    before = entry;
                    entry = entry->next;
                }
                else {
                    // If hash value is equal, then compare the key sizes and the strings.
                    if (likely(this->traits_.key_is_equals(std::forward<key_type>(key),
                                                           entry->pair.first))) {
                        if (likely(before != nullptr))
                            before->next = entry->next;
                        else
                            this->buckets_[index] = entry->next;

                        entry->next = this->freelist_.head();
                        entry->hash = kInvalidHash;
#ifdef _MSC_VER
                        entry->pair.first.clear();
                        entry->pair.second.clear();
#else
                        entry->pair.first = std::string("");
                        entry->pair.second = std::string("");
#endif
                        this->freelist_.set_head(entry);
                        this->freelist_.increase();

                        assert(this->size_ > 0);
                        --(this->size_);

                        this->updateVersion();

                        // Has found the key.
                        return true;
                    }
                }
            }
        }

        // Not found the key.
        return false;
    }
#endif

    static const char * name() {
        switch (HashFunc) {
            case HashFunc_CRC32C:
                return "jstd::dictionary_ex<K, V> (CRC32c)";
            case HashFunc_Time31:
                return "jstd::dictionary_ex<K, V> (Time31)";
            case HashFunc_Time31Std:
                return "jstd::dictionary_ex<K, V> (Time31Std)";
            case HashFunc_SHA1_MSG2:
                return "jstd::dictionary_ex<K, V> (SHA1_Msg2)";
            case HashFunc_SHA1:
                return "jstd::dictionary_ex<K, V> (SHA1)";
            default:
                return "Unknown class name";
        }
    }
}; // dictionary<K, V>

template <typename Key, typename Value, std::size_t HashFunc, typename Traits>
const float basic_dictionary_ex<Key, Value, HashFunc, Traits>::kDefaultLoadFactor = 0.75f;

#if SUPPORT_SSE42_CRC32C
template <typename Key, typename Value>
using dictionary_ex = basic_dictionary_ex<Key, Value, HashFunc_CRC32C>;
#endif

template <typename Key, typename Value>
using dictionary_ex_time31 = basic_dictionary_ex<Key, Value, HashFunc_Time31>;

template <typename Key, typename Value>
using dictionary_ex_time31_std = basic_dictionary_ex<Key, Value, HashFunc_Time31Std>;

#if SUPPORT_SMID_SHA
template <typename Key, typename Value>
using dictionary_ex_sha1_msg2 = basic_dictionary_ex<Key, Value, HashFunc_SHA1_MSG2>;
#endif

#if SUPPORT_SMID_SHA
template <typename Key, typename Value>
using dictionary_ex_sha1 = basic_dictionary_ex<Key, Value, HashFunc_SHA1>;
#endif

} // namespace jstd

#endif // JSTD_DICTIONARY_EX_H
