
#include <stdlib.h>
#include <stdio.h>
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

#define USE_SHA1_HASH           0
#define USE_PICO_HTTP_PARSER    1

#if USE_PICO_HTTP_PARSER
#include <picohttpparser/picohttpparser.h>
#endif

#include "jimi/http_all.h"
#include "jimi/crc32c.h"
#include "jimi/Hash.h"
#include "StopWatch.h"

#include "jimi/jstd/hash_table.h"

using namespace jimi;
using namespace jimi::http;
using namespace TiStore;

#ifdef NDEBUG
static const std::size_t kIterations = 3000000;
#else
static const std::size_t kIterations = 10000;
#endif

#if 1
    static const char * http_header =
        "GET /cookies HTTP/1.1\r\n"
        "Host: 127.0.0.1:8090\r\n"
        "Connection: keep-alive\r\n"
        "Cache-Control: max-age=0\r\n"
        "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n"
        "User-Agent: Mozilla/5.0 (Windows NT 6.1; WOW64) AppleWebKit/537.17 (KHTML, like Gecko) Chrome/24.0.1312.56 Safari/537.17\r\n"
        "Accept-Encoding: gzip,deflate,sdch\r\n"
        "Accept-Language: en-US,en;q=0.8\r\n"
        "Accept-Charset: ISO-8859-1,utf-8;q=0.7,*;q=0.3\r\n"
        "Cookie: name=wookie\r\n"
        "\r\n";
#else
    // Reference from /h2o/picohttpparser/benchmark.c
    static const char * http_header =
        "GET /wp-content/uploads/2010/03/hello-kitty-darth-vader-pink.jpg HTTP/1.1\r\n"
        "Host: www.kittyhell.com\r\n"
        "User-Agent: Mozilla/5.0 (Macintosh; U; Intel Mac OS X 10.6; ja-JP-mac; rv:1.9.2.3) Gecko/20100401 Firefox/3.6.3 Pathtraq/0.9\r\n"
        "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n"
        "Accept-Language: ja,en-us;q=0.7,en;q=0.3\r\n"
        "Accept-Encoding: gzip,deflate\r\n"
        "Accept-Charset: Shift_JIS,utf-8;q=0.7,*;q=0.7\r\n"
        "Keep-Alive: 115\r\n"
        "Connection: keep-alive\r\n"
        "Cookie: wp_ozh_wsa_visits=2; wp_ozh_wsa_visit_lasttime=xxxxxxxxxx; "
        "__utma=xxxxxxxxx.xxxxxxxxxx.xxxxxxxxxx.xxxxxxxxxx.xxxxxxxxxx.x; "
        "__utmz=xxxxxxxxx.xxxxxxxxxx.x.x.utmccn=(referral)|utmcsr=reader.livedoor.com|utmcct=/reader/|utmcmd=referral\r\n"
        "\r\n";
#endif

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
    "X-Powered-By"
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
    "Last-Modified"
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

void stop_watch_test()
{
    StopWatch sw;
    getTickCountStopWatch swTickCount;
    static const std::size_t kSleepIterations = 20;

    StopWatch::time_stamp_t start_time, end_time;
    StopWatch::time_point_t start_point, end_point;
    start_time = StopWatch::timestamp();
    start_point = StopWatch::now();
    sw.start();
    swTickCount.start();
    for (std::size_t i = 0; i < kSleepIterations; ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    swTickCount.stop();
    sw.stop();
    end_time = StopWatch::timestamp();
    end_point = StopWatch::now();

    std::cout << "Iterations:        " << kSleepIterations << std::endl;
    if (sw.getMillisec() != 0.0) {
        std::cout << "Time spent:        " << sw.getMillisec() << " ms" << std::endl;
        std::cout << "Time spent(*):     " << StopWatch::duration(end_time, start_time).millisecs() << " ms" << std::endl;
        std::cout << "Time spent(**):    " << StopWatch::duration(end_point, start_point).millisecs() << " ms" << std::endl;
        std::cout << "Time spent(Tick):  " << swTickCount.getMillisec() << " ms" << std::endl;
    }
    else {
        std::cout << "Time spent:        0.0 ms" << std::endl;
    }
    std::cout << std::endl;
}

void http_parser_test()
{
    StopWatch sw;
    int sum = 0;
    std::size_t request_len = ::strlen(http_header);

    std::cout << "-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=" << std::endl;
    std::cout << "  http_parser_test()" << std::endl;
    std::cout << "-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=" << std::endl;

    sw.start();
    for (std::size_t i = 0; i < kIterations; ++i) {
        http::Parser<1024> http_parser;
        sum += http_parser.parseRequest(http_header, request_len);
    }
    sw.stop();

    std::cout << std::endl;
    std::cout << "Sum:               " << sum << std::endl;
    std::cout << "Length:            " << ::strlen(http_header) << std::endl;
    std::cout << "Iterations:        " << kIterations << std::endl;
    if (sw.getMillisec() != 0.0) {
        std::cout << "Time spent:        " << sw.getMillisec() << " ms" << std::endl;
        std::cout << "Parse speed:       " << (uint64_t)((double)kIterations / sw.getSecond()) << " Parse/Sec" << std::endl;
        std::cout << "Parse throughput:  " << (double)(kIterations * request_len) / sw.getSecond() / (1024.0 * 1024.0) << " MB/Sec" << std::endl;
    }
    else {
        std::cout << "Time spent:        0.0 ms" << std::endl;
        std::cout << "Parse speed:       0   Parse/Sec" << std::endl;
        std::cout << "Parse throughput:  0.0 MB/Sec" << std::endl;
    }
    std::cout << std::endl;
}

void http_parser_ref_test()
{
    StopWatch sw;
    int sum = 0;
    std::size_t request_len = ::strlen(http_header);

    StringRefHelper helper;
    static int mark = 0;

    std::cout << "-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=" << std::endl;
    std::cout << "  http_parser_ref_test()" << std::endl;
    std::cout << "-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=" << std::endl;
    std::cout << std::endl;

    sw.start();
    for (std::size_t i = 0; i < kIterations; ++i) {
        http::ParserRef<1024> http_parser;
        sum += http_parser.parseRequest(http_header, request_len);
#if 0
        if (mark < 3) {
            if (helper.attach(http_parser.getMethodStr())) {
                helper.truncate();
                std::cout << "method = " << http_parser.getMethodStr().c_str() << std::endl;
                helper.recover();
            }
            mark++;
        }
#endif
    }
    sw.stop();

    std::cout << std::endl;
    std::cout << "Sum:               " << sum << std::endl;
    std::cout << "Length:            " << ::strlen(http_header) << std::endl;
    std::cout << "Iterations:        " << kIterations << std::endl;
    if (sw.getMillisec() != 0.0) {
        std::cout << "Time spent:        " << sw.getMillisec() << " ms" << std::endl;
        std::cout << "Parse speed:       " << (uint64_t)((double)kIterations / sw.getSecond()) << " Parse/Sec" << std::endl;
        std::cout << "Parse throughput:  " << (double)(kIterations * request_len) / sw.getSecond() / (1024.0 * 1024.0) << " MB/Sec" << std::endl;
    }
    else {
        std::cout << "Time spent:        0.0 ms" << std::endl;
        std::cout << "Parse speed:       0   Parse/Sec" << std::endl;
        std::cout << "Parse throughput:  0.0 MB/Sec" << std::endl;
    }
    std::cout << std::endl;
}

void crc32c_debug_test()
{
#ifndef NDEBUG
    std::cout << "-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=" << std::endl;
    std::cout << "  crc32c_debug_test()" << std::endl;
    std::cout << "-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=" << std::endl;

    static const size_t kHeaderFieldSize = sizeof(header_fields) / sizeof(char *);
    static const size_t kRepeatTimes = (kIterations / kHeaderFieldSize);

    std::string crc32_str[kHeaderFieldSize];
    StringRef crc32_data[kHeaderFieldSize];
    for (size_t i = 0; i < kHeaderFieldSize; ++i) {
        crc32_str[i].assign(header_fields[i]);
        crc32_data[i].assign(crc32_str[i].c_str(), crc32_str[i].size());
    }

#if CRC32C_IS_X86_64
    {
        std::cout << std::endl;
        std::cout << "crc32c_x64()" << std::endl;
        std::cout << std::endl;

        uint32_t crc32_sum = 0;
        for (size_t i = 0; i < kHeaderFieldSize; ++i) {
            crc32_sum += crc32c_x64(crc32_data[i].c_str(), crc32_data[i].size());

            std::cout << "crc32[";
            std::cout << std::right << std::setw(2) << std::setfill(' ') << std::dec;
            std::cout << i << "]: ";
            std::cout << "0x";
            std::cout << std::right << std::setw(8) << std::setfill('0') << std::hex;
            std::cout << std::setiosflags(std::ios::uppercase);
            std::cout << crc32c_x64(crc32_data[i].c_str(), crc32_data[i].size()) << std::endl;
        }
        std::cout << std::endl;
    }
#endif // CRC32C_IS_X86_64

    {
        std::cout << std::endl;
        std::cout << "crc32c_x32()" << std::endl;
        std::cout << std::endl;

        uint32_t crc32_sum = 0;
        for (size_t i = 0; i < kHeaderFieldSize; ++i) {
            crc32_sum += crc32c_x86(crc32_data[i].c_str(), crc32_data[i].size());

            std::cout << "crc32[";
            std::cout << std::right << std::setw(2) << std::setfill(' ') << std::dec;
            std::cout << i << "]: ";
            std::cout << "0x";
            std::cout << std::right << std::setw(8) << std::setfill('0') << std::hex;
            std::cout << std::setiosflags(std::ios::uppercase);
            std::cout << crc32c_x86(crc32_data[i].c_str(), crc32_data[i].size()) << std::endl;
        }
        std::cout << std::endl;
    }

    std::cout << std::endl;
#endif // NDEBUG, For debug only
}

namespace test {

#define CRC32C_ALGORITHM_IMPL(HashType, Name, Func)             \
    struct Name  {                                              \
        typedef HashType hash_type;                             \
                                                                \
        hash_type crc32c(const char * data, size_t length) {    \
            return Func(data, length);                          \
        }                                                       \
                                                                \
        hash_type crc32c(uint32_t state[5], const char * data,  \
                         size_t length) {                       \
            return 0;                                           \
        }                                                       \
                                                                \
        const char * name() { return "" #Func "()"; }           \
                                                                \
        static const bool isSpecial = false;                    \
    }

#define CRC32C_ALGORITHM_IMPL_EX(HashType, Name, Func)          \
    struct Name {                                               \
        typedef HashType hash_type;                             \
                                                                \
        hash_type crc32c(const char * data, size_t length) {    \
            return 0;                                           \
        }                                                       \
                                                                \
        hash_type crc32c(uint32_t state[5], const char * data,  \
                         size_t length) {                       \
            return Func(state, data, length);                   \
        }                                                       \
                                                                \
        const char * name() { return "" #Func "()"; }           \
                                                                \
        static const bool isSpecial = true;                     \
    }

#if CRC32C_IS_X86_64
CRC32C_ALGORITHM_IMPL(uint32_t, crc32c_x64, jimi::crc32c_x64);
#endif
CRC32C_ALGORITHM_IMPL(uint32_t, crc32c_x86, jimi::crc32c_x86);

#if CRC32C_IS_X86_64
CRC32C_ALGORITHM_IMPL(uint32_t, crc32c_hw_u64, jimi::crc32c_hw_u64);
CRC32C_ALGORITHM_IMPL(uint32_t, crc32c_hw_u64_v2, jimi::crc32c_hw_u64_v2);
#endif
CRC32C_ALGORITHM_IMPL(uint32_t, crc32c_hw_u32, jimi::crc32c_hw_u32);

CRC32C_ALGORITHM_IMPL(uint32_t, sha1_msg2, jimi::sha1_msg2);
CRC32C_ALGORITHM_IMPL_EX(uint32_t, sha1_x86, jimi::sha1_x86);

CRC32C_ALGORITHM_IMPL(uint32_t, Times31, TiStore::hash::Times31);
CRC32C_ALGORITHM_IMPL(uint32_t, Times31_std, TiStore::hash::Times31_std);

} // namespace test

template <typename AlgorithmTy>
void crc32c_benchmark_impl()
{
    typedef typename AlgorithmTy::hash_type hash_type;

    static const size_t kHeaderFieldSize = sizeof(header_fields) / sizeof(char *);
    static const size_t kRepeatTimes = (kIterations / kHeaderFieldSize);

    std::string crc32_str[kHeaderFieldSize];
    StringRef crc32_data[kHeaderFieldSize];
    for (size_t i = 0; i < kHeaderFieldSize; ++i) {
        crc32_str[i].assign(header_fields[i]);
        crc32_data[i].assign(crc32_str[i].c_str(), crc32_str[i].size());
    }

    StopWatch sw;
    hash_type crc32_sum = 0;
    AlgorithmTy algorithm;

    if (AlgorithmTy::isSpecial) {
        sw.start();
        for (size_t i = 0; i < kRepeatTimes; ++i) {
            for (size_t j = 0; j < kHeaderFieldSize; ++j) {
                crc32_sum += algorithm.crc32c(s_sha1_state, crc32_data[j].c_str(), crc32_data[j].size());
            }
        }
        sw.stop();
    }
    else {
        sw.start();
        for (size_t i = 0; i < kRepeatTimes; ++i) {
            for (size_t j = 0; j < kHeaderFieldSize; ++j) {
                crc32_sum += algorithm.crc32c(crc32_data[j].c_str(), crc32_data[j].size());
            }
        }
        sw.stop();
    }

    std::cout << std::endl;
    std::cout << algorithm.name() << std::endl;
    std::cout << std::endl;

    std::cout << "crc32c       : ";
    std::cout << "0x";
    std::cout << std::right << std::setw(8) << std::setfill('0') << std::hex;
    std::cout << std::setiosflags(std::ios::uppercase);
    if (AlgorithmTy::isSpecial)
        std::cout << algorithm.crc32c(s_sha1_state, crc32_data[0].c_str(), crc32_data[0].size()) << std::endl;
    else
        std::cout << algorithm.crc32c(crc32_data[0].c_str(), crc32_data[0].size()) << std::endl;
    std::cout << "crc32c_sum   : ";
    std::cout << std::left << std::setw(0) << std::setfill(' ') << std::dec;
    std::cout << crc32_sum << std::endl;
    std::cout << "elapsed time : ";
    std::cout << std::left << std::setw(0) << std::setfill(' ') << std::setprecision(3) << std::fixed;
    std::cout << sw.getMillisec() << " ms" << std::endl;
}

void crc32c_benchmark()
{
    std::cout << "-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=" << std::endl;
    std::cout << "  crc32c_benchmark()" << std::endl;
    std::cout << "-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=" << std::endl;

    static const size_t kHeaderFieldSize = sizeof(header_fields) / sizeof(char *);
    static const size_t kRepeatTimes = (kIterations / kHeaderFieldSize);

    std::string crc32_str[kHeaderFieldSize];
    StringRef crc32_data[kHeaderFieldSize];
    for (size_t i = 0; i < kHeaderFieldSize; ++i) {
        crc32_str[i].assign(header_fields[i]);
        crc32_data[i].assign(crc32_str[i].c_str(), crc32_str[i].size());
    }

#if CRC32C_IS_X86_64
    crc32c_benchmark_impl<test::crc32c_x64>();
#endif
    crc32c_benchmark_impl<test::crc32c_x86>();

#if CRC32C_IS_X86_64
    crc32c_benchmark_impl<test::crc32c_hw_u64>();
    crc32c_benchmark_impl<test::crc32c_hw_u64_v2>();
#endif
    crc32c_benchmark_impl<test::crc32c_hw_u32>();

#if USE_SHA1_HASH
    crc32c_benchmark_impl<test::sha1_msg2>();
    crc32c_benchmark_impl<test::sha1_x86>();
#endif

    crc32c_benchmark_impl<test::Times31>();
    crc32c_benchmark_impl<test::Times31_std>();

    std::cout << std::endl;
}

namespace test {

class std_map {
public:
    typedef std::map<std::string, std::string> map_type;
    typedef typename map_type::size_type size_type;
    typedef typename map_type::iterator iterator;
    typedef typename map_type::const_iterator const_iterator;

private:
    map_type map_;

public:
    std_map() {}
    ~std_map() {}

    const char * name() { return "std::map<std::string, std::string>"; }
    bool is_hashtable() { return false; }

    iterator begin() { return this->map_.begin(); }
    iterator end() { return this->map_.end(); }

    const_iterator begin() const { return this->map_.begin(); }
    const_iterator end() const { return this->map_.end(); }

    size_type size() const { return this->map_.size(); }
    bool empty() const { return this->map_.empty(); }

    size_type count(const std::string & key) const { return this->map_.count(key); }

    void clear() { this->map_.clear(); }

    void reserve(size_type new_capacity) {
        // Not implemented
    }

    void resize(size_type new_capacity) {
        // Not implemented
    }

    void rehash(size_type new_capacity) {
        // Not implemented
    }

    void shrink_to(size_type new_capacity) {
        // Not implemented
    }

    iterator find(const std::string & key) {
        return this->map_.find(key);
    }

    void insert(const std::string & key, const std::string & value) {
        this->map_.insert(std::make_pair(key, value));
    }

#if 0
    void insert(std::string && key, std::string && value) {
        this->map_.insert(std::make_pair(std::forward<std::string>(key),
                                         std::forward<std::string>(value)));
    }
#endif

    void emplace(const std::string & key, const std::string & value) {
        this->map_.emplace(std::make_pair(key, value));
    }

    void erase(const std::string & key) {
        this->map_.erase(key);
    }

#if 0
    void erase(std::string && key) {
        this->map_.erase(std::forward<std::string>(key));
    }
#endif
};

class std_unordered_map {
public:
    typedef std::unordered_map<std::string, std::string> map_type;
    typedef typename map_type::size_type size_type;
    typedef typename map_type::iterator iterator;
    typedef typename map_type::const_iterator const_iterator;

private:
    map_type map_;

public:
    std_unordered_map() {}
    ~std_unordered_map() {}

    const char * name() { return "std::unordered_map<std::string, std::string>"; }
    bool is_hashtable() { return true; }

    iterator begin() { return this->map_.begin(); }
    iterator end() { return this->map_.end(); }

    const_iterator begin() const { return this->map_.begin(); }
    const_iterator end() const { return this->map_.end(); }

    size_type size() const { return this->map_.size(); }
    bool empty() const { return this->map_.empty(); }

    size_type bucket_count() const { return this->map_.bucket_count(); }

    size_type count(const std::string & key) const { return this->map_.count(key); }

    void clear() { this->map_.clear(); }

    void reserve(size_type max_count) {
        this->map_.reserve(max_count);
    }

    void resize(size_type new_buckets) {
        this->map_.rehash(new_buckets);
    }

    void rehash(size_type new_buckets) {
        this->map_.rehash(new_buckets);
    }

    void shrink_to(size_type new_buckets) {
        this->map_.rehash(new_buckets);
    }

    iterator find(const std::string & key) {
        return this->map_.find(key);
    }

    void insert(const std::string & key, const std::string & value) {
        this->map_.insert(std::make_pair(key, value));
    }

#if 0
    void insert(std::string && key, std::string && value) {
        this->map_.insert(std::make_pair(std::forward<std::string>(key),
                                         std::forward<std::string>(value)));
    }
#endif

    void emplace(const std::string & key, const std::string & value) {
        this->map_.emplace(std::make_pair(key, value));
    }

    void erase(const std::string & key) {
        this->map_.erase(key);
    }

#if 0
    void erase(std::string && key) {
        this->map_.erase(std::forward<std::string>(key));
    }
#endif
};

template <typename T>
class hash_table_impl {
public:
    typedef T map_type;
    typedef typename map_type::key_type key_type;
    typedef typename map_type::value_type value_type;
    typedef typename map_type::size_type size_type;
    typedef typename map_type::iterator iterator;
    typedef typename map_type::const_iterator const_iterator;

private:
    map_type map_;

public:
    hash_table_impl() {}
    ~hash_table_impl() {}

    const char * name() { return map_type::name(); }
    bool is_hashtable() { return true; }

    iterator begin() { return this->map_.begin(); }
    iterator end() { return this->map_.end(); }

    const_iterator begin() const { return this->map_.begin(); }
    const_iterator end() const { return this->map_.end(); }

    size_type size() const { return this->map_.size(); }
    bool empty() const { return this->map_.empty(); }

    size_type bucket_mask() const { return this->map_.bucket_mask(); }
    size_type bucket_count() const { return this->map_.bucket_count(); }

    void clear() { this->map_.clear(); }

    void reserve(size_type new_buckets) {
        this->map_.reserve(new_buckets);
    }

    void resize(size_type new_buckets) {
        this->map_.resize(new_buckets);
    }

    void rehash(size_type new_buckets) {
        this->map_.rehash(new_buckets);
    }

    void shrink_to(size_type new_buckets) {
        this->map_.shrink_to_fast(new_buckets);
    }

    iterator find(const key_type & key) {
        return this->map_.find(key);
    }

    void insert(const key_type & key, const value_type & value) {
        this->map_.insert(std::make_pair(key, value));
    }

#if 0
    void insert(key_type && key, value_type && value) {
        this->map_.insert(std::make_pair(std::forward<key_type>(key),
                                         std::forward<value_type>(value)));
    }
#endif

    void emplace(const std::string & key, const std::string & value) {
        this->map_.emplace(std::make_pair(key, value));
    }

    void erase(const std::string & key) {
        if (key.size() <= 4)
            iterator iter = this->map_.find(key);;
        this->map_.erase(key);
    }

#if 0
    void erase(std::string && key) {
        this->map_.erase(std::forward<std::string>(key));
    }
#endif
};

} // namespace test

template <typename AlgorithmTy>
void hashtable_find_benchmark_impl()
{
    static const size_t kHeaderFieldSize = sizeof(header_fields) / sizeof(char *);
    static const size_t kRepeatTimes = (kIterations / kHeaderFieldSize);

    std::string crc32_str[kHeaderFieldSize];
    StringRef crc32_data[kHeaderFieldSize];
    for (size_t i = 0; i < kHeaderFieldSize; ++i) {
        crc32_str[i].assign(header_fields[i]);
        crc32_data[i].assign(crc32_str[i].c_str(), crc32_str[i].size());
    }

    {
        typedef typename AlgorithmTy::iterator iterator;

        size_t count = 0;
        AlgorithmTy algorithm;
        for (size_t i = 0; i < kHeaderFieldSize; ++i) {
            char buf[16];
#ifdef _MSC_VER
            _itoa_s((int)i, buf, 10);
#else
            sprintf(buf, "%d", (int)i);
#endif
            std::string index = buf;
            algorithm.insert(crc32_str[i], index);
        }

        StopWatch sw;
        sw.start();
        for (size_t i = 0; i < kRepeatTimes; ++i) {
            for (size_t j = 0; j < kHeaderFieldSize; ++j) {
                iterator iter = algorithm.find(crc32_str[j]);
                if (iter != algorithm.end()) {
                    count++;
                }
            }
        }
        sw.stop();

        printf("%s\n\n", algorithm.name());
        printf("count = %" PRIuPTR ", elapsed time: %0.3f ms\n\n", count, sw.getMillisec());
        printf("-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=\n");
        printf("\n");
    }
}

void hashtable_find_benchmark()
{
    std::cout << "-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=" << std::endl;
    std::cout << "  hashtable_find_benchmark()" << std::endl;
    std::cout << "-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=" << std::endl;
    std::cout << std::endl;

    hashtable_find_benchmark_impl<test::std_map>();
    hashtable_find_benchmark_impl<test::std_unordered_map>();

    hashtable_find_benchmark_impl<test::hash_table_impl<jstd::hash_table<std::string, std::string>>>();
#if USE_SHA1_HASH
    hashtable_find_benchmark_impl<test::hash_table_impl<jstd::hash_table_v1<std::string, std::string>>>();
    hashtable_find_benchmark_impl<test::hash_table_impl<jstd::hash_table_v2<std::string, std::string>>>();
#endif
    hashtable_find_benchmark_impl<test::hash_table_impl<jstd::hash_table_v3<std::string, std::string>>>();
    hashtable_find_benchmark_impl<test::hash_table_impl<jstd::hash_table_v4<std::string, std::string>>>();
}

template <typename AlgorithmTy>
void hashtable_rehash_benchmark_impl()
{
    static const size_t kHeaderFieldSize = sizeof(header_fields) / sizeof(char *);
#ifndef NDEBUG
    static const size_t kRepeatTimes = 2;
#else
    static const size_t kRepeatTimes = (kIterations / kHeaderFieldSize);
#endif

    std::string crc32_str[kHeaderFieldSize];
    StringRef crc32_data[kHeaderFieldSize];
    for (size_t i = 0; i < kHeaderFieldSize; ++i) {
        crc32_str[i].assign(header_fields[i]);
        crc32_data[i].assign(crc32_str[i].c_str(), crc32_str[i].size());
    }

    {
        size_t count = 0;
        size_t buckets = 128;

        AlgorithmTy algorithm;
        algorithm.reserve(buckets);

        for (size_t i = 0; i < kHeaderFieldSize; ++i) {
            char buf[16];
#ifdef _MSC_VER
            _itoa_s((int)i, buf, 10);
#else
            sprintf(buf, "%d", (int)i);
#endif
            std::string index = buf;
            algorithm.insert(crc32_str[i], index);
        }

        StopWatch sw;

        sw.start();
        for (size_t i = 0; i < kRepeatTimes; ++i) {
            buckets = 128;
            algorithm.shrink_to(buckets - 1);
#ifndef NDEBUG
            if (algorithm.bucket_count() != buckets) {
                size_t bucket_count = algorithm.bucket_count();
                printf("shrink_to(): size = %" PRIuPTR ", buckets = %" PRIuPTR ", bucket_count = %" PRIuPTR "\n",
                       algorithm.size(), buckets, bucket_count);
            }
#endif
            count += algorithm.bucket_count();
            for (size_t j = 0; j < 7; ++j) {
                buckets *= 2;
                algorithm.rehash(buckets - 1);
#ifndef NDEBUG
                if (algorithm.bucket_count() != buckets) {
                    size_t bucket_count = algorithm.bucket_count();
                    printf("rehash(%u):   size = %" PRIuPTR ", buckets = %" PRIuPTR ", bucket_count = %" PRIuPTR "\n",
                           (uint32_t)j, algorithm.size(), buckets, bucket_count);
                }
#endif
                count += algorithm.bucket_count();
            }
        }
        sw.stop();

        printf("%s\n\n", algorithm.name());
        printf("count = %" PRIuPTR ", elapsed time: %0.3f ms\n\n", count, sw.getMillisec());
        printf("-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=\n");
        printf("\n");
    }
}

void hashtable_rehash_benchmark()
{
    std::cout << "-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=" << std::endl;
    std::cout << "  hashtable_rehash_benchmark()" << std::endl;
    std::cout << "-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=" << std::endl;
    std::cout << std::endl;

    hashtable_rehash_benchmark_impl<test::std_unordered_map>();

    hashtable_rehash_benchmark_impl<test::hash_table_impl<jstd::hash_table<std::string, std::string>>>();
#if USE_SHA1_HASH
    hashtable_rehash_benchmark_impl<test::hash_table_impl<jstd::hash_table_v1<std::string, std::string>>>();
    hashtable_rehash_benchmark_impl<test::hash_table_impl<jstd::hash_table_v2<std::string, std::string>>>();
#endif
    hashtable_rehash_benchmark_impl<test::hash_table_impl<jstd::hash_table_v3<std::string, std::string>>>();
    hashtable_rehash_benchmark_impl<test::hash_table_impl<jstd::hash_table_v4<std::string, std::string>>>();
}

template <typename AlgorithmTy>
void hashtable_insert_benchmark_impl()
{
    static const size_t kHeaderFieldSize = sizeof(header_fields) / sizeof(char *);
#ifndef NDEBUG
    static const size_t kRepeatTimes = 100;
#else
    static const size_t kRepeatTimes = (kIterations / kHeaderFieldSize);
#endif

    std::string crc32_str[kHeaderFieldSize];
    std::string indexs[kHeaderFieldSize];
    StringRef crc32_data[kHeaderFieldSize];
    for (size_t i = 0; i < kHeaderFieldSize; ++i) {
        crc32_str[i].assign(header_fields[i]);
        crc32_data[i].assign(crc32_str[i].c_str(), crc32_str[i].size());
        char buf[16];
#ifdef _MSC_VER
        _itoa_s((int)i, buf, 10);
#else
        sprintf(buf, "%d", (int)i);
#endif
        indexs[i] = buf;
    }

    {
        size_t count = 0;
        AlgorithmTy algorithm;

        StopWatch sw;

        sw.start();
        for (size_t i = 0; i < kRepeatTimes; ++i) {
#if 0
            assert(algorithm.size() == 0);
            algorithm.clear();
            assert(algorithm.size() == 0);
            count += algorithm.size();
#endif
            for (size_t j = 0; j < kHeaderFieldSize; ++j) {
                algorithm.insert(crc32_str[j], indexs[j]);
            }
            count += algorithm.size();

            for (size_t j = 0; j < kHeaderFieldSize; ++j) {
                algorithm.erase(crc32_str[j]);
            }
            assert(algorithm.size() == 0);
            count += algorithm.size();
        }
        sw.stop();

        printf("%s\n\n", algorithm.name());
        printf("count = %" PRIuPTR ", elapsed time: %0.3f ms\n\n", count, sw.getMillisec());
        printf("-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=\n");
        printf("\n");
    }
}

void hashtable_insert_benchmark()
{
    std::cout << "-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=" << std::endl;
    std::cout << "  hashtable_insert_benchmark()" << std::endl;
    std::cout << "-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=" << std::endl;
    std::cout << std::endl;

    hashtable_insert_benchmark_impl<test::std_map>();
    hashtable_insert_benchmark_impl<test::std_unordered_map>();

    hashtable_insert_benchmark_impl<test::hash_table_impl<jstd::hash_table<std::string, std::string>>>();
#if USE_SHA1_HASH
    hashtable_insert_benchmark_impl<test::hash_table_impl<jstd::hash_table_v1<std::string, std::string>>>();
    hashtable_insert_benchmark_impl<test::hash_table_impl<jstd::hash_table_v2<std::string, std::string>>>();
#endif
    hashtable_insert_benchmark_impl<test::hash_table_impl<jstd::hash_table_v3<std::string, std::string>>>();
    hashtable_insert_benchmark_impl<test::hash_table_impl<jstd::hash_table_v4<std::string, std::string>>>();
}

void hashtable_benchmark()
{
    hashtable_find_benchmark();
    hashtable_rehash_benchmark();
    hashtable_insert_benchmark();
}

static uint32_t s_bitmap[65536 / 32 / 2 + 1];

#define BITMAP_GET_U32(bitmap, num)     \
    (uint32_t)(bitmap[(num) >> 6U])

#define BITMAP_CHECK_BITS(bitmap, num)  \
    (uint32_t)(bitmap[(num) >> 6U] & (uint32_t)(1U << (((num) >> 1U) & 0x1FU)))

#define BITMAP_SET_BITS(bitmap, num)  \
    bitmap[(num) >> 6U] |= (uint32_t)(1U << (((num) >> 1U) & 0x1FU))

#define BITMAP_CLEAR_BITS(bitmap, num)  \
    bitmap[(num) >> 6U] &= ~((uint32_t)(1U << (((num) >> 1U) & 0x1FU)))

void generate_prime_65536()
{
    // Clear all bits
    memset((void *)s_bitmap, 0, sizeof(s_bitmap));
    // Set bit for num 1
    s_bitmap[0] = 1;

    for (uint32_t n = 3; n <= 65536U; n += 2) {
        //uint32_t unused = BITMAP_CHECK_BITS(s_bitmap, n);
        uint32_t unused = (uint32_t)(s_bitmap[n >> 6U] & (1U << ((n >> 1U) & 0x1FU)));
        if (unused == 0) {
            for (uint32_t t = n * 3; t <= 65536U; t += n * 2) {
                assert((t & 1) != 0);
                //BITMAP_SET_BITS(s_bitmap, t);
                //s_bitmap[t >> 6U] &= ~((uint32_t)(1U << ((t >> 1U) & 0x1FU)));
                s_bitmap[t >> 6U] |= (uint32_t)(1U << ((t >> 1U) & 0x1FU));
            }
        }
    }
}

int is_prime_u32(uint32_t num) {
    if (num == 2)
        return num;

    if ((num % 2U) == 0U)
        return 0;

    if (num <= 65536) {
        uint32_t unused = BITMAP_CHECK_BITS(s_bitmap, num);
        if (unused == 0)
            return 1;
        else
            return 0;
    }

    if ((num % 3U) == 0U)
        return 0;

    if ((num % 5U) == 0U)
        return 0;

    if ((num % 7U) == 0U)
        return 0;

    if ((num % 11U) == 0U)
        return 0;

    if ((num % 13U) == 0U) {
        return 0;
    }
    else {
        uint32_t max_n = (uint32_t)floor((sqrt((double)num) + 1.0));
        for (uint32_t n = 17U; n <= max_n; n += 2) {
            assert(n <= 65536);
            uint32_t unused = BITMAP_CHECK_BITS(s_bitmap, n);
            if (unused == 0) {
                if ((num % n) == 0) {
                    if (n != num)
                        return 0;
                    else
                        return 1;
                }
            }
        }
    }

    return 1;
}

uint32_t find_first_prime(uint32_t num)
{
    uint32_t prime = num;
    if ((num & (num - 1)) == 0)
        prime++;

    do {
        int isPrime = is_prime_u32(prime);
        if (isPrime == 0)
            prime++;
        else
            return prime;        
    } while (1);

    return 0;
}

uint32_t fast_div_coff(uint32_t dividend, uint32_t k)
{
    uint64_t coeff_m = (1ULL << (32 + k));
    coeff_m = (coeff_m / dividend);
    if ((coeff_m % dividend) != 0)
        coeff_m++;
    return (uint32_t)coeff_m;
}

//
// See: https://stackoverflow.com/questions/5558492/divide-by-10-using-bit-shifts
//
inline
uint32_t fast_div(uint32_t divisor, uint32_t coeff_m, uint32_t shift)
{
    // divisor / dividend = quotient | (remainder)
#if defined(WIN64) || defined(_WIN64) || defined(_M_X64) || defined(_M_AMD64) \
 || defined(__amd64__) || defined(__x86_64__) || defined(__LP64__)
    uint64_t quotient = (uint64_t)coeff_m;
    quotient = (quotient * divisor) >> (32U + shift);
    return (uint32_t)quotient;
#else
    uint64_t quotient = (uint64_t)coeff_m;
    quotient = (quotient * divisor) >> 32U;
    uint32_t quotient32 = ((uint32_t)quotient) >> shift;
    return quotient32;
#endif // __amd64__
}

/*
    x86: Only EAX, ECX, EDX is volatile.
    x64: Only RAX£¬RCX£¬RDX£¬R8£¬R9£¬R10£¬R11 is volatile.

div10 proc 
    mov    edx,1999999Ah    ; load 1/10 * 2^32
    imul   eax              ; edx:eax = dividend / 10 * 2 ^32
    mov    eax,edx          ; eax = dividend / 10
    ret
    endp

    __asm__("divl %2\n"
        : "=d" (remainder), "=a" (quotient)
        : "g" (modulus), "d" (high), "a" (low)
    );
*/


#if defined(__GNUC__) || defined(__clang__) || defined(__linux__)
inline
uint32_t fast_div_asm(uint32_t divisor, uint32_t coeff_m, uint32_t shift)
{
    uint32_t quotient32;
    __asm__ (
        "movl %1, %%ecx\n\t"
        "movl %2, %%eax\n\t"
        "mull %%ecx\n\t"
        "movl %3, %%ecx\n\t"
        "shrl %%cl, %%edx\n\t"
        "movl %%edx, %0"
        : "=r" (quotient32)
        : "r" (divisor), "r" (coeff_m), "r" (shift)
        : "%eax", "%ecx", "%edx", "%cl");
    return quotient32;
}
#elif defined(WIN64) || defined(_WIN64) || defined(_M_X64) || defined(_M_AMD64) \
   || defined(__amd64__) || defined(__x86_64__) || defined(__LP64__)
inline uint32_t fast_div_asm(uint32_t divisor, uint32_t coeff_m, uint32_t shift)
{
    return fast_div(divisor, coeff_m, shift);
}
#else // !x86 mode

/* Local STACK = 12 bytes */
#define STACK           0

/* Local ARGS = 0 bytes */
#define ARGS            0

#define ARG_1           dword ptr [esp +  4 + STACK + ARGS]
#define ARG_2           dword ptr [esp +  8 + STACK + ARGS]
#define ARG_3           dword ptr [esp + 12 + STACK + ARGS]
#define ARG_4           dword ptr [esp + 16 + STACK + ARGS]
#define ARG_5           dword ptr [esp + 20 + STACK + ARGS]
#define ARG_6           dword ptr [esp + 24 + STACK + ARGS]
#define ARG_7           dword ptr [esp + 28 + STACK + ARGS]
#define ARG_8           dword ptr [esp + 32 + STACK + ARGS]
#define ARG_9           dword ptr [esp + 36 + STACK + ARGS]
#define ARG_A           dword ptr [esp + 40 + STACK + ARGS]
#define ARG_B           dword ptr [esp + 44 + STACK + ARGS]
#define ARG_C           dword ptr [esp + 48 + STACK + ARGS]
#define ARG_D           dword ptr [esp + 52 + STACK + ARGS]
#define ARG_E           dword ptr [esp + 56 + STACK + ARGS]
#define ARG_F           dword ptr [esp + 60 + STACK + ARGS]

/* Local ARGS = 0 bytes */
#define LOCAL_ARG1      dword ptr [esp +  0 + STACK]
#define LOCAL_ARG2      dword ptr [esp +  4 + STACK]

inline
__declspec(naked)
uint32_t __cdecl fast_div_asm(uint32_t divisor, uint32_t coeff_m, uint32_t shift)
{
    __asm {
#if defined(ARGS) && (ARGS > 0)
        sub     esp, ARGS      // # Generate Stack Frame
#endif
        mov     ecx, ARG_1
        mov     eax, ARG_2
        mul     ecx
        mov     ecx, ARG_3
        shr     edx, cl
        mov     eax, edx
#if defined(ARGS) && (ARGS > 0)
        add     esp, ARGS
#endif
        ret
    }
}

#undef STACK
#undef ARGS

#undef ARG_1
#undef ARG_2
#undef ARG_3
#undef ARG_4
#undef ARG_5
#undef ARG_6
#undef ARG_7
#undef ARG_8
#undef ARG_9
#undef ARG_A
#undef ARG_B
#undef ARG_C
#undef ARG_D
#undef ARG_E
#undef ARG_F

#undef LOCAL_ARG1
#undef LOCAL_ARG2

#endif // x86 mode

inline
uint32_t fast_div_remainder(uint32_t divisor, uint32_t dividend,
                            uint32_t coeff_m, uint32_t shift)
{
    // divisor / dividend = quotient | (remainder)
#if defined(WIN64) || defined(_WIN64) || defined(_M_X64) || defined(_M_AMD64) \
 || defined(__amd64__) || defined(__x86_64__) || defined(__LP64__)
    uint64_t quotient = (uint64_t)coeff_m;
    quotient = (quotient * divisor) >> (32U + shift);
    uint32_t quotient32 = (uint32_t)quotient;
    assert(divisor >= (uint32_t)(dividend * quotient32));
    return (uint32_t)(divisor - (uint32_t)(dividend * quotient32));
#else
  #if 0
    uint64_t quotient = (uint64_t)coeff_m;
    quotient = (quotient * divisor) >> 32U;
    uint32_t quotient32 = ((uint32_t)quotient) >> shift;
  #else
    uint32_t quotient32 = fast_div(divisor, coeff_m, shift);
  #endif
    assert(divisor >= (uint32_t)(dividend * quotient32));
    return (uint32_t)(divisor - (uint32_t)(dividend * quotient32));
#endif // __amd64__
}

uint32_t unittest_fast_div(uint32_t dividend, uint32_t coeff_m, uint32_t shift)
{
#if 1
    uint32_t sum = 0;
    for (uint32_t n = 1; n < (1U << 31U); ++n) {
        uint32_t n2 = fast_div(n, coeff_m, shift);
        sum += n2;
    }
    return sum;
#else
    for (uint32_t n = 1; n < (1U << 31U); ++n) {
        uint32_t n1 = n / dividend;
        uint32_t n2 = fast_div(n, coeff_m, shift);
        assert(n1 == n2);
        if (n1 != n2) {
            printf("n = %u, n1 = %u, n2 = %u\n", n, n1, n2);
            break;
        }
    }
    return 0;
#endif
}

uint32_t unittest_fast_div_asm(uint32_t dividend, uint32_t coeff_m, uint32_t shift)
{
#if 1
    uint32_t sum = 0;
    for (uint32_t n = 1; n < (1U << 31U); ++n) {
        uint32_t n2 = fast_div_asm(n, coeff_m, shift);
        sum += n2;
    }
    return sum;
#else
    for (uint32_t n = 1; n < (1U << 31U); ++n) {
        uint32_t n1 = n / dividend;
        uint32_t n2 = fast_div_asm(n, coeff_m, shift);
        assert(n1 == n2);
        if (n1 != n2) {
            printf("n = %u, n1 = %u, n2 = %u\n", n, n1, n2);
            break;
        }
    }
    return 0;
#endif
}

uint32_t unittest_fast_div_remainder(uint32_t dividend, uint32_t coeff_m, uint32_t shift)
{
#if 1
    uint32_t sum = 0;
    for (uint32_t n = 1; n < (1U << 31U); ++n) {
        uint32_t n2 = fast_div_remainder(n, dividend, coeff_m, shift);
        sum += n2;
    }
    return sum;
#else
    for (uint32_t n = 1; n < (1U << 31U); ++n) {
        uint32_t n1 = n % dividend;
        uint32_t n2 = fast_div_remainder(n, dividend, coeff_m, shift);
        assert(n1 == n2);
        if (n1 != n2) {
            printf("n = %u, n1 = %u, n2 = %u\n", n, n1, n2);
            break;
        }
    }
    return 0;
#endif
}

void find_power_2_near_prime()
{
    generate_prime_65536();
    printf("\n");

    uint32_t m = fast_div_coff(5, 2);

    uint32_t num = 1;
    for (uint32_t n = 0; n < 31; ++n) {
        uint32_t prime = find_first_prime(num);
        uint32_t m = fast_div_coff(prime, n);
        printf("[%-2u]: num = 0x%08X, prime = %-10u, m = 0x%08X, shift = %u\n", n + 1, num, prime, m, n);
#if 1
        {
            StopWatch sw;
            sw.start();
            uint32_t checksum = unittest_fast_div(prime, m, n);
            sw.stop();
            printf("checksum = %u, time: %0.3f\n", checksum, sw.getMillisec());
        }

        {
            StopWatch sw;
            sw.start();
            uint32_t checksum = unittest_fast_div_asm(prime, m, n);
            sw.stop();
            printf("checksum = %u, time: %0.3f\n", checksum, sw.getMillisec());
        }
#endif
        {
            StopWatch sw;
            sw.start();
            uint32_t checksum = unittest_fast_div_remainder(prime, m, n);
            sw.stop();
            printf("checksum = %u, time: %0.3f\n", checksum, sw.getMillisec());
        }
        num *= 2;
    }

    printf("\n");
}

void http_parser_benchmark()
{
    std::cout << "-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=" << std::endl;
    std::cout << "  http_parser_benchmark()" << std::endl;
    std::cout << "-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=" << std::endl;
    std::cout << std::endl;

    static const int kMaxLoop = 20;
    static std::atomic<int> loop_cnt(0);
    auto request_len = ::strlen(http_header);
    volatile int64_t count = 0;
    volatile int64_t dummy = 0;
    std::thread counter([&] {
        auto last_count = count;
        auto count_ = count;
        auto dummy_ = dummy;
        do {
            std::atomic_thread_fence(std::memory_order_acquire);
            count_ = count;
            dummy_ = dummy;
            std::atomic_thread_fence(std::memory_order_release);
#if 1
            std::cout << std::right << std::setw(10) << std::setfill(' ') << std::dec;
            std::cout << (count_ - last_count);
            std::cout << ", ";
            std::cout << std::right << std::setw(9) << std::setfill(' ') << std::fixed << std::setprecision(3);
            std::cout << (double)((count_ - last_count) * request_len) / 1024.0 / 1024.0 << " MB/Sec";
            std::cout << ",  ";
            std::cout << std::left << std::dec << request_len;
            std::cout << " bytes,  dummy = ";
            std::cout << std::left << std::dec << dummy_;
            std::cout << std::endl;
#else
            printf("%lld,  %0.3f MB/Sec,  %llu bytes,  %lld\n", (count_ - last_count),
                   (double)((count_ - last_count) * request_len) / 1024.0 / 1024.0,
                   request_len, dummy_);
#endif
            last_count = count_;
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            loop_cnt++;
            if (loop_cnt > kMaxLoop) {
                break;
            }
        } while (1);
    });

    http::Parser<1024> http_parser;
    do {
        int64_t dummy_tmp = http_parser.parseRequest(http_header, request_len);
        dummy_tmp += (int64_t)http_parser.getFieldSize();
        http_parser.reset();
        std::atomic_thread_fence(std::memory_order_acquire);
        dummy += dummy_tmp;
        count++;
        std::atomic_thread_fence(std::memory_order_release);
        if (loop_cnt > kMaxLoop) {
            if (counter.joinable()) {
                counter.join();
            }
            break;
        }

    } while (1);

    std::cout << std::endl;
}

void http_parser_ref_benchmark()
{
    std::cout << "-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=" << std::endl;
    std::cout << "  http_parser_ref_benchmark()" << std::endl;
    std::cout << "-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=" << std::endl;
    std::cout << std::endl;

    static const int kMaxLoop = 20;
    static std::atomic<int> loop_cnt(0);
    auto request_len = ::strlen(http_header);
    volatile int64_t count = 0;
    volatile int64_t dummy = 0;
    std::thread counter([&] {
        auto last_count = count;
        auto count_ = count;
        auto dummy_ = dummy;
        do {
            std::atomic_thread_fence(std::memory_order_acquire);
            count_ = count;
            dummy_ = dummy;
            std::atomic_thread_fence(std::memory_order_release);
#if 1
            std::cout << std::right << std::setw(10) << std::setfill(' ') << std::dec;
            std::cout << (count_ - last_count);
            std::cout << ", ";
            std::cout << std::right << std::setw(9) << std::setfill(' ') << std::fixed << std::setprecision(3);
            std::cout << (double)((count_ - last_count) * request_len) / 1024.0 / 1024.0 << " MB/Sec";
            std::cout << ",  ";
            std::cout << std::left << std::dec << request_len;
            std::cout << " bytes,  dummy = ";
            std::cout << std::left << std::dec << dummy_;
            std::cout << std::endl;
#else
            printf("%lld,  %0.3f MB/Sec,  %llu bytes,  %lld\n", (count_ - last_count),
                   (double)((count_ - last_count) * request_len) / 1024.0 / 1024.0,
                   request_len, dummy_);
#endif
            last_count = count_;
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            loop_cnt++;
            if (loop_cnt > kMaxLoop) {
                break;
            }
        } while (1);
    });

    http::ParserRef<1024> http_parser;
    do {
        int64_t dummy_tmp = http_parser.parseRequest(http_header, request_len);
        dummy_tmp += (int64_t)http_parser.getFieldSize();
        http_parser.reset();
        std::atomic_thread_fence(std::memory_order_acquire);
        dummy += dummy_tmp;
        count++;
        std::atomic_thread_fence(std::memory_order_release);
        if (loop_cnt > kMaxLoop) {
            if (counter.joinable()) {
                counter.join();
            }
            break;
        }
    } while (1);

    std::cout << std::endl;
}

#if USE_PICO_HTTP_PARSER

void pico_http_parser_benchmark()
{
    std::cout << "-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=" << std::endl;
    std::cout << "  pico_http_parser_benchmark()" << std::endl;
    std::cout << "-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=" << std::endl;
    std::cout << std::endl;

    static const int kMaxLoop = 20;
    static std::atomic<int> loop_cnt(0);
    auto request_len = ::strlen(http_header);
    volatile int64_t count = 0;
    volatile int64_t dummy = 0;
    std::thread counter([&] {
        auto last_count = count;
        auto count_ = count;
        auto dummy_ = dummy;
        do {
            std::atomic_thread_fence(std::memory_order_acquire);
            count_ = count;
            dummy_ = dummy;
            std::atomic_thread_fence(std::memory_order_release);
#if 1
            std::cout << std::right << std::setw(10) << std::setfill(' ') << std::dec;
            std::cout << (count_ - last_count);
            std::cout << ", ";
            std::cout << std::right << std::setw(9) << std::setfill(' ') << std::fixed << std::setprecision(3);
            std::cout << (double)((count_ - last_count) * request_len) / 1024.0 / 1024.0 << " MB/Sec";
            std::cout << ",  ";
            std::cout << std::left << std::dec << request_len;
            std::cout << " bytes,  dummy = ";
            std::cout << std::left << std::dec << dummy_;
            std::cout << std::endl;
#else
            printf("%lld,  %0.3f MB/Sec,  %llu bytes,  %lld\n", (count_ - last_count),
                   (double)((count_ - last_count) * request_len) / 1024.0 / 1024.0,
                   request_len, dummy_);
#endif
            last_count = count_;
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            loop_cnt++;
            if (loop_cnt > kMaxLoop) {
                break;
            }
        } while (1);
    });

    char * method, * path;
    int pret, minor_version;
    struct phr_header headers[128];
    size_t buflen = request_len + 1, prevbuflen = 0, method_len, path_len, num_headers;

    do {
        /* Parse the request */
        num_headers = sizeof(headers) / sizeof(headers[0]);
        pret = phr_parse_request(http_header, buflen, (const char **)&method, &method_len, (const char **)&path, &path_len,
                                 &minor_version, headers, &num_headers, prevbuflen);
        if (pret > 0) {
            /* successfully parsed the request */
            std::atomic_thread_fence(std::memory_order_acquire);
            dummy += (int64_t)num_headers;
            count++;
            std::atomic_thread_fence(std::memory_order_release);
        }
        else if (pret == -1) {
            //return ParseError;
        }

        /* request is incomplete, continue the loop */
        //assert(pret == -2);

        if (loop_cnt > kMaxLoop) {
            if (counter.joinable()) {
                counter.join();
            }
            break;
        }
    } while (1);

    std::cout << std::endl;
}

#endif // USE_PICO_HTTP_PARSER

int main(int argn, char * argv[])
{
    std::cout << std::endl;
    std::cout << "-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=" << std::endl;
    std::cout << "  Program: http_parser_test" << std::endl;
    std::cout << "-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=" << std::endl;
    std::cout << std::endl;

    http::Parser<1024> http_parser;
    http_parser.parseRequest(http_header, ::strlen(http_header));
    printf("http_parser.getVersion() = %s\n", http_parser.getVersionStr().c_str());
    printf("http_parser.getMethod() = %s\n", http_parser.getMethodStr().c_str());
    printf("http_parser.getURI() = %s\n", http_parser.getURI().c_str());
    printf("\n");
    http_parser.displayFields();
    printf("\n");

#if 0
    //stop_watch_test();
    http_parser_test();
    http_parser_ref_test();
#endif

    crc32c_debug_test();
    crc32c_benchmark();
    //hashtable_benchmark();

    find_power_2_near_prime();

#if 0
    http_parser_benchmark();
    http_parser_ref_benchmark();
#if USE_PICO_HTTP_PARSER
    pico_http_parser_benchmark();
#endif // USE_PICO_HTTP_PARSER
#endif

#ifdef _WIN32
    ::system("pause");
#endif
    return 0;
}
