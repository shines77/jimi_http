
#ifndef JSTD_FIXED_FREELIST_H
#define JSTD_FIXED_FREELIST_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "jimi/basic/stddef.h"
#include "jimi/basic/stdint.h"
#include "jimi/basic/stdsize.h"

#include <memory.h>
#include <assert.h>

namespace jstd {

template <typename T>
class fixed_freelist {
public:
    typedef T           entry_type;
    typedef std::size_t size_type;

private:
    entry_type **   list_;
    size_type       size_;
    size_type       capacity_;

public:
    fixed_freelist() : list_(nullptr), size_(0), capacity_(0) {}
    fixed_freelist(size_type capacity) : list_(nullptr), size_(0), capacity_(0) {
        this->initialize(capacity);
    }
    ~fixed_freelist() {
        if (likely(this->list_ != nullptr)) {
            delete[] this->list_;
            this->list_ = nullptr;
        }
    }

    entry_type ** begin() const { return &this->list_[0]; }
    entry_type ** end() const { return &this->list_[this->size_]; }

    size_type size() const { return this->size_; }
    size_type capacity() const { return this->capacity; }

    size_type data() const { return this->list_; }

    bool is_valid() const { return (this->list_ != nullptr); }
    bool is_empty() const { return (this->size_ == 0); }
    bool is_overflow() const { return (this->size_ >= this->capacity_); }

private:
    void initialize(size_type new_capacity) {
        if (likely(new_capacity > 0)) {
            assert(new_capacity > 0);
            assert((new_capacity & (new_capacity - 1)) == 0);
            entry_type ** new_list = new entry_type *[new_capacity];
            if (likely(new_list != nullptr)) {
                // Initialize the list data.
                memset((void *)new_list, 0, sizeof(entry_type *) * new_capacity);
                // Setting status
                this->list_ = new_list;
                this->size_ = 0;
                this->capacity_ = new_capacity;
            }
        }
    }

public:
    void clear() {
        this->size_ = 0;
    }

    void reserve(size_type new_capacity) {
        if (likely(this->capacity_ != new_capacity)) {
            assert(new_capacity > 0);
            assert((new_capacity & (new_capacity - 1)) == 0);
            entry_type ** new_list = new entry_type *[new_capacity];
            if (likely(new_list != nullptr)) {
                // Initialize the new list data.
                memset((void *)new_list, 0, sizeof(entry_type *) * new_capacity);
                if (likely(this->list_ != nullptr)) {
                    delete[] this->list_;
                }
                // Setting status
                this->list_ = new_list;
                this->size_ = 0;
                this->capacity_ = new_capacity;
            }
        }
    }

    void refill(entry_type * entries, size_type new_capacity) {
        if (likely(this->capacity_ != new_capacity)) {
            assert(new_capacity > 0);
            assert((new_capacity & (new_capacity - 1)) == 0);
            entry_type ** new_list = new entry_type *[new_capacity];
            if (likely(new_list != nullptr)) {
                // Fill the new list data use entries.
                assert(entries != nullptr);
                entry_type * current = entries[new_capacity - 1];
                for (size_type i = 0; i < new_capacity; ++i) {
                    new_list[i] = current;
                    current--;
                }
                if (likely(this->list_ != nullptr)) {
                    delete[] this->list_;
                }
                // Setting status
                this->list_ = new_list;
                this->size_ = new_capacity;
                this->capacity_ = new_capacity;
            }
        }
    }

    void resize(size_type new_capacity) {
        if (likely(this->capacity_ != new_capacity)) {
            assert(new_capacity > 0);
            assert((new_capacity & (new_capacity - 1)) == 0);
            entry_type ** new_list = new entry_type *[new_capacity];
            if (likely(new_list != nullptr)) {
                if (likely(new_capacity > this->capacity_)) {
                    // Copy the data from old list.
                    if (likely(this->list_ != nullptr)) {
                        memcpy((void *)new_list, (const void *)this->list_,
                                sizeof(entry_type *) * this->capacity_);
                        delete[] this->list_;
                    }
                    // Initialize the remain list data.
                    memset((void *)(new_list + this->capacity_), 0,
                            sizeof(entry_type *) * (new_capacity - this->capacity_));
                }
                else {
                    // Copy the data from old list.
                    assert(new_capacity < this->capacity_);
                    if (likely(this->list_ != nullptr)) {
                        memcpy((void *)new_list, (const void *)this->list_,
                                sizeof(entry_type *) * new_capacity);
                        delete[] this->list_;
                    }
                    this->size_ = new_capacity;
                }
                // Setting status
                this->list_ = new_list;
                this->capacity_ = new_capacity;
            }
        }
    }

    entry_type * operator [] (size_type index) {
        assert(this->list_ != nullptr);
        assert(index >= 0 && index < this->size_);
        return this->list_[index];
    }

    void push_back(entry_type * entry) {
        assert(this->list_ != nullptr);
        assert(this->size_ < this->capacity_);
        this->list_[this->size_] = entry;
        ++(this->size_);
        assert(this->size_ <= this->capacity_);
    }

    entry_type * pop_back() {
        assert(this->list_ != nullptr);
        assert(this->size_ > 0);
        --(this->size_);
        entry_type * entry = this->list_[this->size_];
        return entry;
    }

    void safe_push_back(entry_type * entry) {
        if (likely(this->list_ != nullptr)) {
            if (likely(this->size_ < this->capacity_)) {
                this->list_[this->size_] = entry;
                ++(this->size_);
                assert(this->size_ <= this->capacity_);
            }
        }
    }

    entry_type * safe_pop_back() {
        entry_type * entry;
        if (likely(this->list_ != nullptr)) {
            if (likely(this->size_ > 0)) {
                --(this->size_);
                entry_type * entry = this->list_[this->size_];
                return entry;
            }
        }
        return nullptr;
    }
};

} // namespace jstd

#endif // JSTD_FIXED_FREELIST_H
