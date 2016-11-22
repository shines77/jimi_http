
#include <stdlib.h>
#include <stdio.h>

#include "jimi_http/http_all.h"
#include "stop_watch.h"

using namespace jimi::http;

#ifdef NDEBUG
static const std::size_t kIterations = 5000000;
#else
static const std::size_t kIterations = 100000;
#endif

void test_http_parser()
{
    const char * http_request = "GET /cookies HTTP/1.1\r\n"
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

    StdStopWatch sw;
    int sum = 0;
    std::size_t request_len = ::strlen(http_request);
    sw.start();
    for (int i = 0; i < kIterations; ++i) {
        HttpParser http_parser;
        sum += http_parser.parse(http_request, request_len);
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
    const char * http_request = "GET /cookies HTTP/1.1\r\n"
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
    HttpParser http_parser;
    printf("http_parser.getHttpVersion() = %u\n", http_parser.getHttpVersion());
    printf("http_parser.getRequestMethod() = %u\n", http_parser.getRequestMethod());
    http_parser.parse(http_request, ::strlen(http_request));
    printf("\n");
    http_parser.diplayEntries();
    printf("\n");

    test_http_parser();

#ifdef _WIN32
    ::system("pause");
#endif
    return 0;
}
