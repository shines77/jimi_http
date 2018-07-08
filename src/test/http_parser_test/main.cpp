
#include <stdlib.h>
#include <stdio.h>
#include "jimi/basic/stdint.h"
#include "jimi/basic/stdsize.h"
#include "jimi/basic/inttypes.h"
#include <string.h>

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
#include "jimi/crc32.h"
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

void crc32_benchmark()
{
    std::cout << "-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=" << std::endl;
    std::cout << "  crc32_benchmark()" << std::endl;
    std::cout << "-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=" << std::endl;

    static const size_t kHeaderFieldSize = sizeof(header_fields) / sizeof(char *);
    static const size_t kRepeatTimes = (kIterations / kHeaderFieldSize);

    std::string crc32_str[kHeaderFieldSize];
    StringRef crc32_data[kHeaderFieldSize];
    for (size_t i = 0; i < kHeaderFieldSize; ++i) {
        crc32_str[i].assign(header_fields[i]);
        crc32_data[i].assign(crc32_str[i].c_str(), crc32_str[i].size());
    }

#ifndef NDEBUG
#if CRC32C_IS_X86_64
    {
        std::cout << std::endl;
        std::cout << "crc32_x64()" << std::endl;
        std::cout << std::endl;

        uint32_t crc32_sum = 0;
        for (size_t i = 0; i < kHeaderFieldSize; ++i) {
            crc32_sum += crc32_x64(crc32_data[i].c_str(), crc32_data[i].size());

            std::cout << "crc32[";
            std::cout << std::right << std::setw(2) << std::setfill(' ') << std::dec;
            std::cout << i << "]: ";
            std::cout << "0x";
            std::cout << std::right << std::setw(8) << std::setfill('0') << std::hex;
            std::cout << std::setiosflags(std::ios::uppercase);
            std::cout << crc32_x64(crc32_data[i].c_str(), crc32_data[i].size()) << std::endl;
        }
        std::cout << std::endl;
    }
#endif // CRC32C_IS_X86_64

    {
        std::cout << std::endl;
        std::cout << "crc32_x32()" << std::endl;
        std::cout << std::endl;

        uint32_t crc32_sum = 0;
        for (size_t i = 0; i < kHeaderFieldSize; ++i) {
            crc32_sum += crc32_x86(crc32_data[i].c_str(), crc32_data[i].size());

            std::cout << "crc32[";
            std::cout << std::right << std::setw(2) << std::setfill(' ') << std::dec;
            std::cout << i << "]: ";
            std::cout << "0x";
            std::cout << std::right << std::setw(8) << std::setfill('0') << std::hex;
            std::cout << std::setiosflags(std::ios::uppercase);
            std::cout << crc32_x86(crc32_data[i].c_str(), crc32_data[i].size()) << std::endl;
        }
        std::cout << std::endl;
    }
#endif // For debug only

#if CRC32C_IS_X86_64
    {
        StopWatch sw;
        uint32_t crc32_sum = 0;
        sw.start();
        for (size_t i = 0; i < kRepeatTimes; ++i) {
            for (size_t j = 0; j < kHeaderFieldSize; ++j) {
                crc32_sum += crc32_x64(crc32_data[j].c_str(), crc32_data[j].size());
            }
        }
        sw.stop();

        std::cout << std::endl;
        std::cout << "crc32_x64()" << std::endl;
        std::cout << std::endl;

        std::cout << "crc32        : ";
        std::cout << "0x";
        std::cout << std::right << std::setw(8) << std::setfill('0') << std::hex;
        std::cout << std::setiosflags(std::ios::uppercase);
        std::cout << crc32_x64(crc32_data[0].c_str(), crc32_data[0].size()) << std::endl;
        std::cout << "crc32_sum    : ";
        std::cout << std::left << std::setw(0) << std::setfill(' ') << std::dec;
        std::cout << crc32_sum << std::endl;
        std::cout << "elapsed time : ";
        std::cout << std::left << std::setw(0) << std::setfill(' ') << std::setprecision(3) << std::fixed;
        std::cout << sw.getMillisec() << " ms" << std::endl;
    }
#endif // CRC32C_IS_X86_64

    {
        StopWatch sw;
        uint32_t crc32_sum = 0;
        sw.start();
        for (size_t i = 0; i < kRepeatTimes; ++i) {
            for (size_t j = 0; j < kHeaderFieldSize; ++j) {
                crc32_sum += crc32_x86(crc32_data[j].c_str(), crc32_data[j].size());
            }
        }
        sw.stop();

        std::cout << std::endl;
        std::cout << "crc32_x86()" << std::endl;
        std::cout << std::endl;

        std::cout << "crc32        : ";
        std::cout << "0x";
        std::cout << std::right << std::setw(8) << std::setfill('0') << std::hex;
        std::cout << std::setiosflags(std::ios::uppercase);
        std::cout << crc32_x86(crc32_data[0].c_str(), crc32_data[0].size()) << std::endl;
        std::cout << "crc32_sum    : ";
        std::cout << std::left << std::setw(0) << std::setfill(' ') << std::dec;
        std::cout << crc32_sum << std::endl;
        std::cout << "elapsed time : ";
        std::cout << std::left << std::setw(0) << std::setfill(' ') << std::setprecision(3) << std::fixed;
        std::cout << sw.getMillisec() << " ms" << std::endl;
    }

#if CRC32C_IS_X86_64
    {
        StopWatch sw;
        uint32_t crc32_sum = 0;
        sw.start();
        for (size_t i = 0; i < kRepeatTimes; ++i) {
            for (size_t j = 0; j < kHeaderFieldSize; ++j) {
                crc32_sum += intel_crc32_u64(crc32_data[j].c_str(), crc32_data[j].size());
            }
        }
        sw.stop();

        std::cout << std::endl;
        std::cout << "intel_crc32_u64()" << std::endl;
        std::cout << std::endl;

        std::cout << "crc32        : ";
        std::cout << "0x";
        std::cout << std::right << std::setw(8) << std::setfill('0') << std::hex;
        std::cout << std::setiosflags(std::ios::uppercase);
        std::cout << intel_crc32_u64(crc32_data[0].c_str(), crc32_data[0].size()) << std::endl;
        std::cout << "crc32_sum    : ";
        std::cout << std::left << std::setw(0) << std::setfill(' ') << std::dec;
        std::cout << crc32_sum << std::endl;
        std::cout << "elapsed time : ";
        std::cout << std::left << std::setw(0) << std::setfill(' ') << std::setprecision(3) << std::fixed;
        std::cout << sw.getMillisec() << " ms" << std::endl;
    }
#endif // CRC32C_IS_X86_64

#if CRC32C_IS_X86_64
    {
        StopWatch sw;
        uint32_t crc32_sum = 0;
        sw.start();
        for (size_t i = 0; i < kRepeatTimes; ++i) {
            for (size_t j = 0; j < kHeaderFieldSize; ++j) {
                crc32_sum += intel_crc32_u64_v2(crc32_data[j].c_str(), crc32_data[j].size());
            }
        }
        sw.stop();

        std::cout << std::endl;
        std::cout << "intel_crc32_u64_v2()" << std::endl;
        std::cout << std::endl;

        std::cout << "crc32        : ";
        std::cout << "0x";
        std::cout << std::right << std::setw(8) << std::setfill('0') << std::hex;
        std::cout << std::setiosflags(std::ios::uppercase);
        std::cout << intel_crc32_u64_v2(crc32_data[0].c_str(), crc32_data[0].size()) << std::endl;
        std::cout << "crc32_sum    : ";
        std::cout << std::left << std::setw(0) << std::setfill(' ') << std::dec;
        std::cout << crc32_sum << std::endl;
        std::cout << "elapsed time : ";
        std::cout << std::left << std::setw(0) << std::setfill(' ') << std::setprecision(3) << std::fixed;
        std::cout << sw.getMillisec() << " ms" << std::endl;
    }
#endif // CRC32C_IS_X86_64

    {
        StopWatch sw;
        uint32_t crc32_sum = 0;
        sw.start();
        for (size_t i = 0; i < kRepeatTimes; ++i) {
            for (size_t j = 0; j < kHeaderFieldSize; ++j) {
                crc32_sum += intel_crc32_u32(crc32_data[j].c_str(), crc32_data[j].size());
            }
        }
        sw.stop();

        std::cout << std::endl;
        std::cout << "intel_crc32_u32()" << std::endl;
        std::cout << std::endl;

        std::cout << "crc32        : ";
        std::cout << "0x";
        std::cout << std::right << std::setw(8) << std::setfill('0') << std::hex;
        std::cout << std::setiosflags(std::ios::uppercase);
        std::cout << intel_crc32_u32(crc32_data[0].c_str(), crc32_data[0].size()) << std::endl;
        std::cout << "crc32_sum    : ";
        std::cout << std::left << std::setw(0) << std::setfill(' ') << std::dec;
        std::cout << crc32_sum << std::endl;
        std::cout << "elapsed time : ";
        std::cout << std::left << std::setw(0) << std::setfill(' ') << std::setprecision(3) << std::fixed;
        std::cout << sw.getMillisec() << " ms" << std::endl;
    }

#if USE_SHA1_HASH
    {
        StopWatch sw;
        uint32_t hash32_sum = 0;
        sw.start();
        for (size_t i = 0; i < kRepeatTimes; ++i) {
            for (size_t j = 0; j < kHeaderFieldSize; ++j) {
                hash32_sum += jimi::sha1_msg2(crc32_data[j].c_str(), crc32_data[j].size());
            }
        }
        sw.stop();

        std::cout << std::endl;
        std::cout << "jimi::sha1_msg2()" << std::endl;
        std::cout << std::endl;

        std::cout << "hash32       : ";
        std::cout << "0x";
        std::cout << std::right << std::setw(8) << std::setfill('0') << std::hex;
        std::cout << std::setiosflags(std::ios::uppercase);
        std::cout << jimi::sha1_msg2(crc32_data[0].c_str(), crc32_data[0].size()) << std::endl;
        std::cout << "hash32_sum   : ";
        std::cout << std::left << std::setw(0) << std::setfill(' ') << std::dec;
        std::cout << hash32_sum << std::endl;
        std::cout << "elapsed time : ";
        std::cout << std::left << std::setw(0) << std::setfill(' ') << std::setprecision(3) << std::fixed;
        std::cout << sw.getMillisec() << " ms" << std::endl;
    }
#endif

#if USE_SHA1_HASH
    {
        StopWatch sw;
        uint32_t hash32_sum = 0;
        //alignas(16) uint32_t sha1_state[5];

        sw.start();
        for (size_t i = 0; i < kRepeatTimes; ++i) {
            for (size_t j = 0; j < kHeaderFieldSize; ++j) {
                //memcpy((void *)&sha1_state[0], (const void *)&jimi::s_sha1_state[0], sizeof(uint32_t) * 5);
                hash32_sum += jimi::sha1_x86(jimi::s_sha1_state, crc32_data[j].c_str(), crc32_data[j].size());
            }
        }
        sw.stop();

        std::cout << std::endl;
        std::cout << "jimi::sha1_x86()" << std::endl;
        std::cout << std::endl;

        //memcpy((void *)&sha1_state[0], (const void *)&jimi::s_sha1_state[0], sizeof(uint32_t) * 5);

        std::cout << "hash32       : ";
        std::cout << "0x";
        std::cout << std::right << std::setw(8) << std::setfill('0') << std::hex;
        std::cout << std::setiosflags(std::ios::uppercase);
        std::cout << jimi::sha1_x86(jimi::s_sha1_state, crc32_data[0].c_str(), crc32_data[0].size()) << std::endl;
        std::cout << "hash32_sum   : ";
        std::cout << std::left << std::setw(0) << std::setfill(' ') << std::dec;
        std::cout << hash32_sum << std::endl;
        std::cout << "elapsed time : ";
        std::cout << std::left << std::setw(0) << std::setfill(' ') << std::setprecision(3) << std::fixed;
        std::cout << sw.getMillisec() << " ms" << std::endl;
    }
#endif

    {
        StopWatch sw;
        uint32_t hash32_sum = 0;
        sw.start();
        for (size_t i = 0; i < kRepeatTimes; ++i) {
            for (size_t j = 0; j < kHeaderFieldSize; ++j) {
                hash32_sum += TiStore::hash::Times31(crc32_data[j].c_str(), crc32_data[j].size());
            }
        }
        sw.stop();

        std::cout << std::endl;
        std::cout << "TiStore::hash::Times31()" << std::endl;
        std::cout << std::endl;

        std::cout << "hash32       : ";
        std::cout << "0x";
        std::cout << std::right << std::setw(8) << std::setfill('0') << std::hex;
        std::cout << std::setiosflags(std::ios::uppercase);
        std::cout << TiStore::hash::Times31(crc32_data[0].c_str(), crc32_data[0].size()) << std::endl;
        std::cout << "hash32_sum   : ";
        std::cout << std::left << std::setw(0) << std::setfill(' ') << std::dec;
        std::cout << hash32_sum << std::endl;
        std::cout << "elapsed time : ";
        std::cout << std::left << std::setw(0) << std::setfill(' ') << std::setprecision(3) << std::fixed;
        std::cout << sw.getMillisec() << " ms" << std::endl;
    }

    {
        StopWatch sw;
        uint32_t hash32_sum = 0;
        sw.start();
        for (size_t i = 0; i < kRepeatTimes; ++i) {
            for (size_t j = 0; j < kHeaderFieldSize; ++j) {
                hash32_sum += TiStore::hash::Times31_std(crc32_data[j].c_str(), crc32_data[j].size());
            }
        }
        sw.stop();

        std::cout << std::endl;
        std::cout << "TiStore::hash::Times31_std()" << std::endl;
        std::cout << std::endl;

        std::cout << "hash32       : ";
        std::cout << "0x";
        std::cout << std::right << std::setw(8) << std::setfill('0') << std::hex;
        std::cout << std::setiosflags(std::ios::uppercase);
        std::cout << TiStore::hash::Times31_std(crc32_data[0].c_str(), crc32_data[0].size()) << std::endl;
        std::cout << "hash32_sum   : ";
        std::cout << std::left << std::setw(0) << std::setfill(' ') << std::dec;
        std::cout << hash32_sum << std::endl;
        std::cout << "elapsed time : ";
        std::cout << std::left << std::setw(0) << std::setfill(' ') << std::setprecision(3) << std::fixed;
        std::cout << sw.getMillisec() << " ms" << std::endl;
    }

    std::cout << std::endl;
}

void hashtable_benchmark()
{
    std::cout << "-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=" << std::endl;
    std::cout << "  hashtable_benchmark()" << std::endl;
    std::cout << "-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=" << std::endl;
    std::cout << std::endl;

    static const size_t kHeaderFieldSize = sizeof(header_fields) / sizeof(char *);
    static const size_t kRepeatTimes = (kIterations / kHeaderFieldSize);

    std::string crc32_str[kHeaderFieldSize];
    StringRef crc32_data[kHeaderFieldSize];
    for (size_t i = 0; i < kHeaderFieldSize; ++i) {
        crc32_str[i].assign(header_fields[i]);
        crc32_data[i].assign(crc32_str[i].c_str(), crc32_str[i].size());
    }

    {
        size_t count = 0;
        typedef std::map<std::string, std::string>::iterator map_iterator;
        std::map<std::string, std::string> map;
        for (size_t i = 0; i < kHeaderFieldSize; ++i) {
            char buf[16];
#ifdef _MSC_VER
            _itoa_s((int)i, buf, 10);
#else
            sprintf(buf, "%d", (int)i);
#endif
            std::string index = buf;
            map.insert(std::make_pair(crc32_str[i], index));
        }

        StopWatch sw;
        sw.start();
        for (size_t i = 0; i < kRepeatTimes; ++i) {
            for (size_t j = 0; j < kHeaderFieldSize; ++j) {
                map_iterator iter = map.find(crc32_str[j]);
                if (iter != map.end()) {
                    count++;
                }
            }
        }
        sw.stop();

        printf("std::map<std::string, std::string>\n\n");
        printf("count = %" PRIuPTR ", elapsed time: %0.3f ms\n\n", count, sw.getMillisec());
        printf("-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=\n");
        printf("\n");
    }

    {
        size_t count = 0;
        typedef std::unordered_map<std::string, std::string>::iterator map_iterator;
        std::unordered_map<std::string, std::string> table;
        for (size_t i = 0; i < kHeaderFieldSize; ++i) {
            char buf[16];
#ifdef _MSC_VER
            _itoa_s((int)i, buf, 10);
#else
            sprintf(buf, "%d", (int)i);
#endif
            std::string index = buf;
            table.insert(std::make_pair(crc32_str[i], index));
        }

        StopWatch sw;
        sw.start();
        for (size_t i = 0; i < kRepeatTimes; ++i) {
            for (size_t j = 0; j < kHeaderFieldSize; ++j) {
                map_iterator iter = table.find(crc32_str[j]);
                if (iter != table.end()) {
                    count++;
                }
            }
        }
        sw.stop();

        printf("std::unordered_map<std::string, std::string>\n\n");
        printf("count = %" PRIuPTR ", elapsed time: %0.3f ms\n\n", count, sw.getMillisec());
        printf("-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=\n");
        printf("\n");
    }

    {
        size_t count = 0;
        typedef jstd::hash_table<std::string, std::string>::iterator iterator;
        jstd::hash_table<std::string, std::string> table;
        for (size_t i = 0; i < kHeaderFieldSize; ++i) {
            char buf[16];
#ifdef _MSC_VER
            _itoa_s((int)i, buf, 10);
#else
            sprintf(buf, "%d", (int)i);
#endif
            std::string index = buf;
            table.insert(crc32_str[i], index);
        }

        StopWatch sw;
        sw.start();
        for (size_t i = 0; i < kRepeatTimes; ++i) {
            for (size_t j = 0; j < kHeaderFieldSize; ++j) {
                iterator iter = table.find(crc32_str[j]);
                if (iter != table.end()) {
                    count++;
                }
            }
        }
        sw.stop();

        printf("jstd::hash_table<std::string, std::string>\n\n");
        printf("count = %" PRIuPTR ", elapsed time: %0.3f ms\n\n", count, sw.getMillisec());
        printf("-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=\n");
        printf("\n");
    }

#if USE_SHA1_HASH
    {
        size_t count = 0;
        typedef jstd::hash_table_v1<std::string, std::string>::iterator iterator;
        jstd::hash_table_v1<std::string, std::string> table;
        for (size_t i = 0; i < kHeaderFieldSize; ++i) {
            char buf[16];
#ifdef _MSC_VER
            _itoa_s((int)i, buf, 10);
#else
            sprintf(buf, "%d", (int)i);
#endif
            std::string index = buf;
            table.insert(crc32_str[i], index);
        }

        StopWatch sw;
        sw.start();
        for (size_t i = 0; i < kRepeatTimes; ++i) {
            for (size_t j = 0; j < kHeaderFieldSize; ++j) {
                iterator iter = table.find(crc32_str[j]);
                if (iter != table.end()) {
                    count++;
                }
            }
        }
        sw.stop();

        printf("jstd::hash_table_v1<std::string, std::string>\n\n");
        printf("count = %" PRIuPTR ", elapsed time: %0.3f ms\n\n", count, sw.getMillisec());
        printf("-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=\n");
        printf("\n");
    }
#endif

#if USE_SHA1_HASH
    {
        size_t count = 0;
        typedef jstd::hash_table_v2<std::string, std::string>::iterator iterator;
        jstd::hash_table_v2<std::string, std::string> table;
        for (size_t i = 0; i < kHeaderFieldSize; ++i) {
            char buf[16];
#ifdef _MSC_VER
            _itoa_s((int)i, buf, 10);
#else
            sprintf(buf, "%d", (int)i);
#endif
            std::string index = buf;
            table.insert(crc32_str[i], index);
        }

        StopWatch sw;
        sw.start();
        for (size_t i = 0; i < kRepeatTimes; ++i) {
            for (size_t j = 0; j < kHeaderFieldSize; ++j) {
                iterator iter = table.find(crc32_str[j]);
                if (iter != table.end()) {
                    count++;
                }
            }
        }
        sw.stop();

        printf("jstd::hash_table_v2<std::string, std::string>\n\n");
        printf("count = %" PRIuPTR ", elapsed time: %0.3f ms\n\n", count, sw.getMillisec());
        printf("-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=\n");
        printf("\n");
    }
#endif

    {
        size_t count = 0;
        typedef jstd::hash_table_v3<std::string, std::string>::iterator iterator;
        jstd::hash_table_v3<std::string, std::string> table;
        for (size_t i = 0; i < kHeaderFieldSize; ++i) {
            char buf[16];
#ifdef _MSC_VER
            _itoa_s((int)i, buf, 10);
#else
            sprintf(buf, "%d", (int)i);
#endif
            std::string index = buf;
            table.insert(crc32_str[i], index);
        }

        StopWatch sw;
        sw.start();
        for (size_t i = 0; i < kRepeatTimes; ++i) {
            for (size_t j = 0; j < kHeaderFieldSize; ++j) {
                iterator iter = table.find(crc32_str[j]);
                if (iter != table.end()) {
                    count++;
                }
            }
        }
        sw.stop();

        printf("jstd::hash_table_v3<std::string, std::string>\n\n");
        printf("count = %" PRIuPTR ", elapsed time: %0.3f ms\n\n", count, sw.getMillisec());
        printf("-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=\n");
        printf("\n");
    }

    {
        size_t count = 0;
        typedef jstd::hash_table_v4<std::string, std::string>::iterator iterator;
        jstd::hash_table_v4<std::string, std::string> table;
        for (size_t i = 0; i < kHeaderFieldSize; ++i) {
            char buf[16];
#ifdef _MSC_VER
            _itoa_s((int)i, buf, 10);
#else
            sprintf(buf, "%d", (int)i);
#endif
            std::string index = buf;
            table.insert(crc32_str[i], index);
        }

        StopWatch sw;
        sw.start();
        for (size_t i = 0; i < kRepeatTimes; ++i) {
            for (size_t j = 0; j < kHeaderFieldSize; ++j) {
                iterator iter = table.find(crc32_str[j]);
                if (iter != table.end()) {
                    count++;
                }
            }
        }
        sw.stop();

        printf("jstd::hash_table_v4<std::string, std::string>\n\n");
        printf("count = %" PRIuPTR ", elapsed time: %0.3f ms\n\n", count, sw.getMillisec());
        printf("-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=\n");
        printf("\n");
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

#if 1
    //stop_watch_test();
    http_parser_test();
    http_parser_ref_test();
#endif

    crc32_benchmark();
    hashtable_benchmark();

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
