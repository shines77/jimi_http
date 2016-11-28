
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include <sstream>
#include <iostream>
#include <iomanip>
#include <atomic>
#include <thread>

#include "jimi_http/http_all.h"
#include "stop_watch.h"

using namespace jimi::http;

#ifdef NDEBUG
static const std::size_t kIterations = 5000000;
#else
static const std::size_t kIterations = 100000;
#endif

void http_parser_benchmark()
{
    const char * http_header = "GET /cookies HTTP/1.1\r\n"
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
	auto request_len = ::strlen(http_header);
	volatile int64_t count = 0;
    volatile int64_t dummy = 0;
	std::thread counter([&] {
		auto last_count = count;
		auto count_ = count;
		do {
			count_ = count;
            std::atomic_thread_fence(std::memory_order_acquire);
#if 1
            std::cout << std::right << std::setw(10) << std::setfill(' ') << std::dec;
            std::cout << (count_ - last_count);
            std::cout << ", ";
			std::cout << std::right << std::setw(9) << std::setfill(' ') << std::fixed << std::setprecision(3);
            std::cout << (double)((count_ - last_count) * request_len) / 1024.0 / 1024.0 << " MB/Sec";
            std::cout << ",  dummy = ";
            std::cout << std::left << std::oct << dummy;
            std::cout << std::endl;
#else
            printf("%lld, %0.3f MB/Sec, %lld\n", (count_ - last_count), (double)((count_ - last_count) * request_len) / 1024.0 / 1024.0, dummy);
#endif
            std::atomic_thread_fence(std::memory_order_release);

			last_count = count_;
			std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		} while (1);
	});

    HttpParser<1024> parser;
	do {
        std::atomic_thread_fence(std::memory_order_acquire);
		dummy += parser.parse(http_header, ::strlen(http_header));
        std::atomic_thread_fence(std::memory_order_acquire);
        //dummy += parser.getEntrySize();
        parser.reset();
		count++;
        std::atomic_thread_fence(std::memory_order_release);
	} while (1);
}

void http_parser_test()
{
    const char * http_header = "GET /cookies HTTP/1.1\r\n"
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
    StopWatch sw;
    int sum = 0;
    std::size_t request_len = ::strlen(http_header);
    sw.start();
    for (std::size_t i = 0; i < kIterations; ++i) {
        HttpParser<1024> http_parser;
        sum += http_parser.parse(http_header, request_len);
    }
    sw.stop();

    std::cout << "Sum:               " << sum << std::endl;
    std::cout << "Iterations:        " << kIterations << std::endl;
    if (sw.getElapsedMillisec() != 0.0) {
        std::cout << "Time spent:        " << sw.getElapsedMillisec() << " ms" << std::endl;
        std::cout << "Parse speed:       " << (uint64_t)((double)kIterations / sw.getElapsedSecond()) << " Parse/Sec" << std::endl;
        std::cout << "Parse throughput:  " << (double)(kIterations * request_len) / sw.getElapsedSecond() / (1024.0 * 1024.0) << " MB/Sec" << std::endl;
    }
    else {
        std::cout << "Time spent:        0.0 ms" << std::endl;
        std::cout << "Parse speed:       0   Parse/Sec" << std::endl;
        std::cout << "Parse throughput:  0.0 MB/Sec" << std::endl;
    }
    std::cout << std::endl;
}

int main(int argn, char * argv[])
{
    const char * http_header = "GET /cookies HTTP/1.1\r\n"
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

    printf("http_parser_test\n\n");
    HttpParser<1024> http_parser;
    printf("http_parser.getHttpVersion() = %u\n", http_parser.getHttpVersion());
    printf("http_parser.getRequestMethod() = %u\n", http_parser.getRequestMethod());
    http_parser.parse(http_header, ::strlen(http_header));
    printf("\n");
    http_parser.displayEntries();
    printf("\n");

#if 0
    http_parser_test();
#else
    http_parser_benchmark();
#endif

#ifdef _WIN32
    ::system("pause");
#endif
    return 0;
}
