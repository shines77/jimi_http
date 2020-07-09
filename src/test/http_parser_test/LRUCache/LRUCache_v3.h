
#ifndef LEETCODE_LRUCACHE_V3_H
#define LEETCODE_LRUCACHE_V3_H

#if defined(_MSC_VER) && (_MSC_VER > 1200)
#pragma once
#endif

#include <stdint.h>

#include "LRUNode.h"
#include "LRUHashTable.h"
#include "LinkedList.h"

#include "jimi/jstd/hash_table.h"
#include "jimi/jstd/hash_map.h"
#include "jimi/jstd/hash_map_ex.h"
#include "jimi/jstd/dictionary.h"

#include <unordered_map>

#if USE_JSTD_DICTIONARY

//
// Problem title: 146. LRU Cache
// Problem URL: https://leetcode.com/problems/lru-cache/
//

namespace LeetCode {
namespace V3 {

template <typename KeyT, typename ValueT, ValueT kFailedValue = LRUValue::FailedValue>
class LRUCacheBase {
public:
    typedef KeyT    key_type;
    typedef ValueT  value_type;

    typedef std::size_t                             size_type;
    typedef LRUNode<key_type, value_type>           node_type;
#if SUPPORT_SSE42_CRC32C
    typedef jstd::dictionary<key_type, node_type *>
                                                    hash_table_type;
#else
    typedef jstd::dictionary_time31<key_type, node_type *>
                                                    hash_table_type;
#endif
    typedef typename hash_table_type::iterator      hash_iterator;
    typedef ContinuousDoubleLinkedList<node_type>   linkedlist_type;

    static const size_type kDefaultCapacity = 32;

private:
    size_type       capacity_;
    linkedlist_type list_;
    hash_table_type cache_;

public:
    LRUCacheBase() : capacity_(kDefaultCapacity), list_(kDefaultCapacity), cache_(kDefaultCapacity) {
    }

    LRUCacheBase(size_t capacity) : capacity_(capacity), list_(capacity), cache_(capacity) {
    }

    ~LRUCacheBase() {
        capacity_ = 0;
    }

    size_type sizes() const { return list_.sizes(); }
    size_type capacity() const { return list_.capacity(); }

protected:
    void add_new(const key_type & key, const value_type & value) {
        node_type * new_node = list_.push_front_fast(key, value);
        if (new_node) {
            cache_.insert(key, new_node);
        }
    }

    void touch(const key_type & key, const value_type & value) {
        // Pop the tail node.
        node_type * tail = list_.back();
        if (tail != list_.head()) {
            //list_.pop_back();
            if (key != tail->key) {
                // Remove the old key from the hash table.
                cache_.erase(tail->key);
                // Insert the new key and value to the hash table.
                cache_.insert(key, tail);
            }
            // Save the new key and value.
            tail->key = key;
            tail->value = value;
            // Push the tail node to head again.
            //list_.push_front(tail);
            list_.move_to_front(tail);
        }
    }

public:
    value_type get(const key_type & key) {
        hash_iterator iter = cache_.find(key);
        if (iter != cache_.end()) {
            node_type * node = iter->pair.second;
            assert(node != nullptr);
            assert(key == node->key);
            list_.move_to_front(node);
            return node->value;
        }
        return kFailedValue;
    }

    void put(const key_type & key, const value_type & value) {
        hash_iterator iter = cache_.find(key);
        if (iter != cache_.end()) {
            node_type * node = iter->pair.second;
            assert(node != nullptr);
            assert(key == node->key);
            node->value = value;
            list_.move_to_front(node);
        }
        else {
            if (list_.size() >= capacity_) {
                touch(key, value);
            }
            else {
                add_new(key, value);
            }
        }
    }

    node_type * begin() {
        return list_.begin();
    }

    node_type * end() {
        return list_.end();
    }

    void display() {
        list_.display();
    }
};

typedef LRUCacheBase<int, int> LRUCache;

} // namespace V3
} // namespace LeetCode

#endif // USE_JSTD_DICTIONARY

#endif // LEETCODE_LRUCACHE_V3_H
