
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

template <typename CharT, std::size_t InitCapacity>
class BasicStringRefList {
public:
    typedef CharT char_type;
    typedef std::size_t size_type;
    typedef std::basic_string<char_type> std_string;
    typedef BasicStringRef<char_type> string_ref;

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
        std::size_t capacity;
        std::size_t size;
        EntryPair * entries;

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
    StringRef ref;
private:
    size_type capacity_;
    size_type size_;
    EntryChunk * head_;
    EntryChunk * tail_;
public:
    EntryPair items[kInitCapacity];

public:
    BasicStringRefList(std::size_t capacity = kInitCapacity)
        : ref(), capacity_(capacity), size_(0), head_(nullptr), tail_(nullptr) { initList(); }
    BasicStringRefList(const char_type * data)
        : ref(data),  capacity_(kInitCapacity), size_(0), head_(nullptr), tail_(nullptr) { initList(); }
    BasicStringRefList(const char_type * data, size_type size)
        : ref(data, size), capacity_(kInitCapacity), size_(0), head_(nullptr), tail_(nullptr) { initList(); }
    template <size_type N>
    BasicStringRefList(char_type (&src)[N])
        : ref(src), capacity_(kInitCapacity), size_(0), head_(nullptr), tail_(nullptr) { initList(); }
    BasicStringRefList(const std_string & src)
        : ref(src), capacity_(kInitCapacity), size_(0), head_(nullptr), tail_(nullptr) { initList(); }
    BasicStringRefList(const string_ref & src)
        : ref(src), capacity_(kInitCapacity), size_(0), head_(nullptr), tail_(nullptr) { initList(); }

    ~BasicStringRefList() {
        freeEntryChunks();
    }

private:
    void append_item(std::size_t index, const char * key, std::size_t key_len,
                     const char * value, std::size_t value_len) {
        EntryPair & item = items[index];
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
        ref.clear();
        capacity_ = kInitCapacity;
        size_ = 0;
        head_ = nullptr;
        tail_ = nullptr;
        freeEntryChunks();
    }

    void clear() {
        reset();
    }

    void setRef(const char_type * data) {
        ref.assign(data);
    }

    void setRef(const char_type * data, size_type size) {
        ref.assign(data, size);
    }

    void setRef(const std_string & src) {
        ref.assign(src);
    }

    template <size_type N>
    void setRef(char_type (&src)[N]) {
        ref.assign(src);
    }

    void initList() {
#ifndef NDEBUG
        ::memset((void *)&items, 0, sizeof(items));
#endif // !NDEBUG
    }

    void freeEntryChunks() {
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
            append_item(size_, key, key_len, value, value_len);
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
            // TODO: If the item count more than fixed kInitCapacity size,
            // append to the last position of last chunk.
            //
            append_item(size_, key, key_len, value, value_len);
            size_++;
        }
    }
};

template <std::size_t InitCapacity>
using StringRefListA = BasicStringRefList<char, InitCapacity>;

template <std::size_t InitCapacity>
using StringRefListW = BasicStringRefList<wchar_t, InitCapacity>;

template <std::size_t InitCapacity>
using StringRefList  = StringRefListA<InitCapacity>;

} // namespace jimi

#endif // !JIMI_HTTP_STRINGREFLIST_H
