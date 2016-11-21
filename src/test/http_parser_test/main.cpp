
#include <stdlib.h>
#include <stdio.h>

#include "jimi_http/http_all.h"

using namespace jimi::http;

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
#ifdef _WIN32
    ::system("pause");
#endif
    return 0;
}
