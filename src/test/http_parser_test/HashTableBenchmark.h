
#ifndef HASHTABLE_BENCHMARK_H
#define HASHTABLE_BENCHMARK_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <stdlib.h>
#include <stdio.h>
#include "jimi/basic/stddef.h"
#include "jimi/basic/stdint.h"
#include "jimi/basic/stdsize.h"
#include "jimi/basic/inttypes.h"
#include <string.h>
#include <math.h>

#include <sstream>
#include <iostream>
#include <iomanip>
#include <atomic>
#include <thread>
#include <chrono>
#include <string>
#include <map>
#include <unordered_map>

#include "jimi/http_all.h"
#include "jimi/crc32c.h"
#include "jimi/Hash.h"
#include "jimi/support/StopWatch.h"

#include "jimi/jstd/hash_table.h"
#include "jimi/jstd/hash_map.h"
#include "jimi/jstd/hash_map_ex.h"
#include "jimi/jstd/dictionary.h"

namespace testing {

//
// See: https://blog.csdn.net/janekeyzheng/article/details/42419407
//
static const char * header_fields[] = {
    // Request
    "Accept",
    "Accept-Charset",
    "Accept-Encoding",
    "Accept-Language",
    "Authorization",
    "Cache-Control",
    "Connection",
    "Cookie",
    "Content-Length",
    "Content-MD5",
    "Content-Type",
    "Date",
    "DNT",
    "From",
    "Front-End-Https",
    "Host",
    "If-Match",
    "If-Modified-Since",
    "If-None-Match",
    "If-Range",
    "If-Unmodified-Since",
    "Max-Forwards",
    "Pragma",
    "Proxy-Authorization",
    "Range",
    "Referer",
    "User-Agent",
    "Upgrade",
    "Via",
    "Warning",
    "X-ATT-DeviceId",
    "X-Content-Type-Options",
    "X-Forwarded-For",
    "X-Forwarded-Proto",
    "X-Powered-By",
    "X-Requested-With",
    "X-XSS-Protection",

    // Response
    "Access-Control-Allow-Origin",
    "Accept-Ranges",
    "Age",
    "Allow",
    //"Cache-Control",
    //"Connection",
    "Content-Encoding",
    "Content-Language",
    //"Content-Length",
    "Content-Disposition",
    //"Content-MD5",
    "Content-Range",
    //"Content-Type",
    "Date",
    "ETag",
    "Expires",
    "Last-Modified",
    "Link",
    "Location",
    "P3P",
    "Proxy-Authenticate",
    "Refresh",
    "Retry-After",
    "Server",
    "Set-Cookie",
    "Strict-Transport-Security",
    "Trailer",
    "Transfer-Encoding",
    "Vary",
    "Via",
    "WWW-Authenticate",
    //"X-Content-Type-Options",
    //"X-Powered-By",
    //"X-XSS-Protection",

    "Last"
};

} // namespace testing

class Testing {
private:
    std::string name_;

public:
    Testing(char * name) : name_(name) {}
    virtual ~Testing() {}

    const std::string & name() const {
        return name_;
    }

    const char * c_str() const {
        return name_.c_str();
    }

    virtual void run() = 0;
};

template <typename MapTy>
struct HashTableAlgorithm {
    static std::string s_name;

    static const std::string & name() {
        return s_name;
    }
};

template <>
struct HashTableAlgorithm<std::map<std::string, std::string>> {
    static const std::string name() {
        return "std::map<K, V>";
    }
};

template <>
struct HashTableAlgorithm<std::unordered_map<std::string, std::string>> {
    static const std::string name() {
        return "std::unordered_map<K, V>";
    }
};

template <typename MapTy>
std::string HashTableAlgorithm<MapTy>::s_name = "HashTable<K, V>";

class HashTableBenchmark {
private:
#ifdef NDEBUG
    static const size_t kIterations = 3000000;
#else
    static const size_t kIterations = 5000;
#endif

    static const size_t kHeaderFieldSize = sizeof(testing::header_fields) / sizeof(char *);
    static const size_t kRepeatTimes = (kIterations / kHeaderFieldSize);

    std::string fields_[kHeaderFieldSize];
    std::string values_[kHeaderFieldSize];

public:
    HashTableBenchmark() {
        initFields();
    }

    virtual ~HashTableBenchmark() {
        // TODO:
    }

    void initFields() {
        for (size_t i = 0; i < kHeaderFieldSize; i++) {
            fields_[i].assign(testing::header_fields[i]);
            char buf[16];
#ifdef _MSC_VER
            _itoa_s((int)i, buf, 10);
#else
            sprintf(buf, "%d", (int)i);
#endif
            values_[i] = buf;
        }
    }

    template <typename MapTy>
    void StlMap_FindBenchmark() {
        typedef MapTy map_type;
        typedef typename map_type::iterator iterator;

        map_type map;
        for (size_t i = 0; i < kHeaderFieldSize; ++i) {
            map.emplace(fields_[i], values_[i]);
        }

        size_t checksum = 0;

        StopWatch sw;
        sw.start();
        for (size_t i = 0; i < kRepeatTimes; ++i) {
            for (size_t j = 0; j < kHeaderFieldSize; ++j) {
                iterator iter = map.find(fields_[j]);
                if (iter != map.end()) {
                    checksum++;
                }
            }
        }
        sw.stop();

        printf("-------------------------------------------------------------------------\n");
        printf(" %-36s  ", HashTableAlgorithm<MapTy>::name().c_str());
        printf("sum = %-10" PRIuPTR "  time: %8.3f ms\n", checksum, sw.getMillisec());
    }

    template <typename HashTableTy>
    void HashTable_FindBenchmark() {
        typedef HashTableTy hashtable_type;
        typedef typename hashtable_type::iterator iterator;

        hashtable_type hashTable;
        for (size_t i = 0; i < kHeaderFieldSize; ++i) {
            hashTable.emplace(fields_[i], values_[i]);
        }

        size_t checksum = 0;

        StopWatch sw;
        sw.start();
        for (size_t i = 0; i < kRepeatTimes; ++i) {
            for (size_t j = 0; j < kHeaderFieldSize; ++j) {
                iterator iter = hashTable.find(fields_[j]);
                if (iter != hashTable.end()) {
                    checksum++;
                }
            }
        }
        sw.stop();

        printf("-------------------------------------------------------------------------\n");
        printf(" %-36s  ", HashTableTy::name());
        printf("sum = %-10" PRIuPTR "  time: %8.3f ms\n", checksum, sw.getMillisec());
    }

    void runFindBenchmark() {
        std::cout << "-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=" << std::endl;
        std::cout << "  HashTable_FindBenchmark()" << std::endl;
        std::cout << "-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=" << std::endl;
        std::cout << std::endl;

        StlMap_FindBenchmark<std::map<std::string, std::string>>();
        StlMap_FindBenchmark<std::unordered_map<std::string, std::string>>();

#if SUPPORT_SSE42_CRC32C
        HashTable_FindBenchmark<jstd::hash_table<std::string, std::string>>();
#endif
        HashTable_FindBenchmark<jstd::hash_table_time31<std::string, std::string>>();
        HashTable_FindBenchmark<jstd::hash_table_time31_std<std::string, std::string>>();
#if SUPPORT_SMID_SHA
        HashTable_FindBenchmark<jstd::hash_table_sha1_msg2<std::string, std::string>>();
        HashTable_FindBenchmark<jstd::hash_table_sha1<std::string, std::string>>();
#endif

#if USE_JSTD_HASH_MAP
#if SUPPORT_SSE42_CRC32C
        HashTable_FindBenchmark<jstd::hash_map<std::string, std::string>>();
#endif
        HashTable_FindBenchmark<jstd::hash_map_time31<std::string, std::string>>();
        HashTable_FindBenchmark<jstd::hash_map_time31_std<std::string, std::string>>();
#if SUPPORT_SMID_SHA
        HashTable_FindBenchmark<jstd::hash_map_sha1_msg2<std::string, std::string>>();
        HashTable_FindBenchmark<jstd::hash_map_sha1<std::string, std::string>>();
#endif
#endif // USE_JSTD_HASH_MAP

#if USE_JSTD_HASH_MAP_EX
#if SUPPORT_SSE42_CRC32C
        HashTable_FindBenchmark<jstd::hash_map_ex<std::string, std::string>>();
#endif
        HashTable_FindBenchmark<jstd::hash_map_ex_time31<std::string, std::string>>();
        HashTable_FindBenchmark<jstd::hash_map_ex_time31_std<std::string, std::string>>();
#if SUPPORT_SMID_SHA
        HashTable_FindBenchmark<jstd::hash_map_ex_sha1_msg2<std::string, std::string>>();
        HashTable_FindBenchmark<jstd::hash_map_ex_sha1<std::string, std::string>>();
#endif
#endif // USE_JSTD_HASH_MAP_EX

#if USE_JSTD_DICTIONARY
#if SUPPORT_SSE42_CRC32C
        HashTable_FindBenchmark<jstd::dictionary<std::string, std::string>>();
#endif
        HashTable_FindBenchmark<jstd::dictionary_time31<std::string, std::string>>();
        HashTable_FindBenchmark<jstd::dictionary_time31_std<std::string, std::string>>();
#if SUPPORT_SMID_SHA
        HashTable_FindBenchmark<jstd::dictionary_sha1_msg2<std::string, std::string>>();
        HashTable_FindBenchmark<jstd::dictionary_sha1<std::string, std::string>>();
#endif
#endif // USE_JSTD_DICTIONARY

        std::cout << "-------------------------------------------------------------------------" << std::endl;
        std::cout << std::endl;
    }

    void run() {
        // HashTable::find(key)
        runFindBenchmark();
    }
};

#endif // HASHTABLE_BENCHMARK_H
