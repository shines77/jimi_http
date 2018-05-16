
#ifndef JIMI_HTTP_STRINGREFLIST_H
#define JIMI_HTTP_STRINGREFLIST_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <stdint.h>
#include <string.h>
#include <memory.h>
#include <assert.h>
#include <cstddef>
#include <string>

#include "jimi/basic/stddef.h"
#include "jimi/StringRef.h"

namespace jimi {

template <typename CharTy, std::size_t InitCapacity>
class BasicStringRefList {
public:
    typedef CharTy char_type;
    typedef std::size_t size_type;
    typedef std::basic_string<char_type> string_type;
    typedef BasicStringRef<char_type> stringref_type;

    static const std::size_t kInitCapacity = InitCapacity;

private:
    struct Entry {
        uint32_t offset;
        uint32_t length;
    };

    struct EntryPair {
        Entry key;
        Entry value;
    };

    struct EntryChunk {
        EntryChunk * next;
        std::size_t  capacity;
        std::size_t  size;
        EntryPair *  entries;

        EntryChunk(std::size_t _capacity)
            : next(nullptr), capacity(_capacity), size(0), entries(nullptr) {
        }
        ~EntryChunk() {
            next = nullptr;
            if (entries) {
                delete[] entries;
                capacity = 0;
                size = 0;
                entries = nullptr;
            }
        }
    };

public:
    stringref_type ref;
private:
    size_type capacity_;
    size_type size_;
    EntryChunk * head_;
    EntryChunk * tail_;
public:
    EntryPair items[kInitCapacity];

public:
    BasicStringRefList(std::size_t capacity = kInitCapacity)
        : ref(), capacity_(capacity), size_(0), head_(nullptr), tail_(nullptr) {
        initList();
    }
    BasicStringRefList(const char_type * data)
        : ref(data),  capacity_(kInitCapacity), size_(0), head_(nullptr), tail_(nullptr) {
        initList();
    }
    BasicStringRefList(const char_type * data, size_type size)
        : ref(data, size), capacity_(kInitCapacity), size_(0), head_(nullptr), tail_(nullptr) {
        initList();
    }
    template <size_type N>
    BasicStringRefList(const char_type (&src)[N])
        : ref(src, N - 1), capacity_(kInitCapacity), size_(0), head_(nullptr), tail_(nullptr) {
        initList();
    }
    BasicStringRefList(const string_type & src)
        : ref(src), capacity_(kInitCapacity), size_(0), head_(nullptr), tail_(nullptr) {
        initList();
    }
    BasicStringRefList(const stringref_type & src)
        : ref(src), capacity_(kInitCapacity), size_(0), head_(nullptr), tail_(nullptr) {
        initList();
    }

    ~BasicStringRefList() {
        destroyChunks();
    }

private:
    void appendItem(std::size_t index,
                    const char * key, std::size_t key_len,
                    const char * value, std::size_t value_len) {
        EntryPair & item = this->items[index];
        item.key.offset = static_cast<uint16_t>(key - ref.data());
        item.key.length = static_cast<uint16_t>(key_len);
        item.value.offset = static_cast<uint16_t>(value - ref.data());
        item.value.length = static_cast<uint16_t>(value_len);
    }

public:
    const char_type * data() const { return ref.data(); }
    size_type capacity() const { return capacity_; }
    size_type size() const { return size_; }

    const char_type * c_str() const { return data(); }
    size_type length() const { return size(); }

    bool is_empty() const { return (size() == 0); }

    void reset() {
        this->ref.clear();
        capacity_ = kInitCapacity;
        size_ = 0;
        head_ = nullptr;
        tail_ = nullptr;
        destroyChunks();
    }

    void clear() {
        reset();
    }

    void setRef(const char_type * data) {
        this->ref.assign(data);
    }

    void setRef(const char_type * data, size_type size) {
        this->ref.assign(data, size);
    }

    void setRef(const char_type * first, const char_type * last) {
        this->ref.assign(first, last);
    }

    template <size_type N>
    void setRef(const char_type (&src)[N]) {
        this->ref.assign(src, N - 1);
    }

    void setRef(const string_type & src) {
        this->ref.assign(src);
    }

    void setRef(const stringref_type & src) {
        this->ref.assign(src);
    }

    void initList() {
#ifndef NDEBUG
        ::memset((void *)&items, 0, sizeof(items));
#endif // !NDEBUG
    }

    void destroyChunks() {
        EntryChunk * chunk = head_;
        while (chunk != nullptr) {
            EntryChunk * next = chunk->next;
            delete chunk;
            chunk = next;
        }
        head_ = nullptr;
        tail_ = nullptr;
    }

    EntryChunk * findLastChunk() {
        EntryChunk * chunk = head_;
        while (chunk != nullptr) {
            if (chunk->next != nullptr)
                chunk = chunk->next;
            else
                break;
        }
        return chunk;
    }

    void append(const char * key, std::size_t key_len,
                const char * value, std::size_t value_len) {
        assert(key != nullptr);
        assert(value != nullptr);
        if (likely(size_ < kInitCapacity)) {
            assert(size_ < capacity_);
            appendItem(size_, key, key_len, value, value_len);
            size_++;
        }
        else {
            if (unlikely(size_ >= capacity_)) {
                // Add a new enttries chunk
                static const std::size_t kChunkSize = 32;
                EntryChunk * newChunk = new EntryChunk(kChunkSize);
                if (newChunk != nullptr) {
                    if (head_ == nullptr)
                        head_ = newChunk;
                    if (tail_ == nullptr) {
                        tail_ = newChunk;
                    }
                    else {
                        tail_->next = newChunk;
                        tail_ = newChunk;
                    }
                }
                capacity_ += kChunkSize;
            }
            assert(size_ < capacity_);
            //
            // TODO: If the item count more than fixed kInitCapacity size,
            //       append to the last position of last chunk.
            //
            appendItem(size_, key, key_len, value, value_len);
            size_++;
        }
    }
};

template <std::size_t InitCapacity>
using StringRefListA = BasicStringRefList<char, InitCapacity>;

template <std::size_t InitCapacity>
using StringRefListW = BasicStringRefList<wchar_t, InitCapacity>;

template <std::size_t InitCapacity>
using StringRefList  = BasicStringRefList<char, InitCapacity>;

} // namespace jimi

#endif // JIMI_HTTP_STRINGREFLIST_H
