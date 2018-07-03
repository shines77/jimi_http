
#ifndef JIMI_HTTP_CRC32_H
#define JIMI_HTTP_CRC32_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "jimi/basic/stddef.h"
#include "jimi/basic/stdint.h"
#include "jimi/basic/stdsize.h"

#include <assert.h>

#ifdef __SSE4_2__
#ifdef _MSC_VER
#include <nmmintrin.h>  // For SSE 4.2
#else
#include <x86intrin.h>
//#include <nmmintrin.h>  // For SSE 4.2
#endif
#endif

#if defined(WIN64) || defined(_WIN64) || defined(_M_X64) || defined(_M_AMD64) \
 || defined(__amd64__) || defined(__x86_64__)
#ifndef __IS_X86_64
#define __IS_X86_64     1
#endif
#endif // _WIN64 || __amd64__

namespace jimi {

static uint32_t crc32_x86(const char * data, size_t length)
{
    assert(data != nullptr);

    static const size_t kStepLen = sizeof(uint32_t);
    static const uint32_t kOneMask = 0xFFFFFFFFUL;

    uint32_t crc32 = 0;

    uint32_t data32;
    const char * data_end = data + length;
    ssize_t remain = length;

    do {
        if (likely(remain >= (ssize_t)kStepLen)) {
            data32 = *(uint32_t *)(data);
            crc32 = _mm_crc32_u32(crc32, data32);
            data += kStepLen;
            remain -= kStepLen;
        }
        else {
            assert(data_end >= data);
            data32 = *(uint32_t *)(data);
            size_t rest = (size_t)((ssize_t)kStepLen - (data_end - data));
            assert(rest > 0 && rest <= kStepLen);
            uint32_t mask = kOneMask >> (rest * 8U);
            data32 &= mask;
            crc32 = _mm_crc32_u32(crc32, data32);
            break;
        }
    } while (1);

    return crc32;
}

#if __IS_X86_64

static uint32_t crc32_x64(const char * data, size_t length)
{
    assert(data != nullptr);

    static const size_t kStepLen = sizeof(uint64_t);
    static const uint64_t kOneMask = 0xFFFFFFFFFFFFFFFFULL;

    uint64_t crc64 = 0;

    uint64_t data64;
    const char * data_end = data + length;
    ssize_t remain = length;

    do {
        if (likely(remain >= (ssize_t)kStepLen)) {
            data64 = *(uint64_t *)(data);
            crc64 = _mm_crc32_u64(crc64, data64);
            data += kStepLen;
            remain -= kStepLen;
        }
        else {
            assert(data_end >= data);
            data64 = *(uint64_t *)(data);
            size_t rest = (size_t)((ssize_t)kStepLen - (data_end - data));
            assert(rest > 0 && rest <= kStepLen);
            uint64_t mask = kOneMask >> (rest * 8U);
            data64 &= mask;
            crc64 = _mm_crc32_u64(crc64, data64);
            break;
        }
    } while (1);

    return (uint32_t)crc64;
}

#endif // __IS_X86_64

#if __IS_X86_64

static uint32_t intel_crc32_u64(const char * data, size_t length)
{
    assert(data != nullptr);
    uint64_t crc64 = 0;

    static const size_t kStepLen = sizeof(uint64_t);
    uint64_t * src = (uint64_t *)data;
    uint64_t * src_end = src + (length / kStepLen);

    while (likely(src < src_end)) {
        crc64 = _mm_crc32_u64(crc64, *src);
        ++src;
    }

    uint32_t crc32 = (uint32_t)crc64;

    size_t i = length / kStepLen * kStepLen;
    while (likely(i < length)) {
        crc32 = _mm_crc32_u8(crc32, data[i]);
        ++i;
    }
    return crc32;
}

#endif // __IS_X86_64

#if __IS_X86_64

static uint32_t intel_crc32_u64_v2(const char * data, size_t length)
{
    assert(data != nullptr);
    uint64_t crc64 = 0;

    static const size_t kStepLen = sizeof(uint64_t);
    uint64_t * src = (uint64_t *)data;
    uint64_t * src_end = src + (length / kStepLen);

    while (likely(src < src_end)) {
        crc64 = _mm_crc32_u64(crc64, *src);
        ++src;
    }

    uint32_t crc32 = (uint32_t)crc64;
    unsigned char * src8 = (unsigned char *)src;
    unsigned char * src8_end = (unsigned char *)(data + length);

    while (likely(src8 < src8_end)) {
        crc32 = _mm_crc32_u8(crc32, *src8);
        ++src8;
    }
    return crc32;
}

#endif // __IS_X86_64

static uint32_t intel_crc32_u32(const char * data, size_t length)
{
    assert(data != nullptr);
    uint32_t crc32 = 0;

    static const size_t kStepLen = sizeof(uint32_t);
    uint32_t * src = (uint32_t *)data;
    uint32_t * src_end = src + (length / kStepLen);

    while (likely(src < src_end)) {
        crc32 = _mm_crc32_u32(crc32, *src);
        ++src;
    }

    unsigned char * src8 = (unsigned char *)src;
    unsigned char * src8_end = (unsigned char *)(data + length);

    while (likely(src8 < src8_end)) {
        crc32 = _mm_crc32_u8(crc32, *src8);
        ++src8;
    }
    return crc32;
}

} // namespace jimi

#endif // JIMI_HTTP_CRC32_H
