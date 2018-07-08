
#ifndef JIMI_HTTP_CRC32C_H
#define JIMI_HTTP_CRC32C_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "jimi/basic/stddef.h"
#include "jimi/basic/stdint.h"
#include "jimi/basic/stdsize.h"

#include <assert.h>

#ifndef __SSE4_2__
#define __SSE4_2__      1
#endif // __SSE4_2__

#ifdef __SSE4_2__
#ifdef _MSC_VER
#include <nmmintrin.h>  // For SSE 4.2
#include <immintrin.h>  // For SSE 3
#include <emmintrin.h>  // For SSE 2
#else
#include <x86intrin.h>
//#include <nmmintrin.h>  // For SSE 4.2
#endif // _MSC_VER
#endif // __SSE4_2__

#if defined(WIN64) || defined(_WIN64) || defined(_M_X64) || defined(_M_AMD64) \
 || defined(__amd64__) || defined(__x86_64__) || defined(__LP64__)
#ifndef CRC32C_IS_X86_64
#define CRC32C_IS_X86_64    1
#endif
#endif // _WIN64 || __amd64__

namespace jimi {

static uint32_t crc32_x86(const char * data, size_t length)
{
    assert(data != nullptr);

    static const ssize_t kStepSize = sizeof(uint32_t);
    static const uint32_t kMaskOne = 0xFFFFFFFFUL;
    const char * data_end = data + length;

    uint32_t crc32 = ~0;
    ssize_t remain = length;

    do {
        if (likely(remain >= kStepSize)) {
            assert(data < data_end);
            uint32_t data32 = *(uint32_t *)(data);
            crc32 = _mm_crc32_u32(crc32, data32);
            data += kStepSize;
            remain -= kStepSize;
        }
        else {
            assert((data_end - data) >= 0 && (data_end - data) < kStepSize);
            assert((data_end - data) == remain);
            assert(remain >= 0);
            if (likely(remain > 0)) {
                uint32_t data32 = *(uint32_t *)(data);
                uint32_t rest = (uint32_t)(kStepSize - remain);
                assert(rest > 0 && rest < (uint32_t)kStepSize);
                uint32_t mask = kMaskOne >> (rest * 8U);
                data32 &= mask;
                crc32 = _mm_crc32_u32(crc32, data32);
            }
            break;
        }
    } while (1);

    return ~crc32;
}

static uint32_t crc32_x64(const char * data, size_t length)
{
#if CRC32C_IS_X86_64
    assert(data != nullptr);

    static const ssize_t kStepSize = sizeof(uint64_t);
    static const uint64_t kMaskOne = 0xFFFFFFFFFFFFFFFFULL;
    const char * data_end = data + length;

    uint64_t crc64 = ~0;
    ssize_t remain = length;

    do {
        if (likely(remain >= kStepSize)) {
            assert(data < data_end);
            uint64_t data64 = *(uint64_t *)(data);
            crc64 = _mm_crc32_u64(crc64, data64);
            data += kStepSize;
            remain -= kStepSize;
        }
        else {
            assert((data_end - data) >= 0 && (data_end - data) < kStepSize);
            assert((data_end - data) == remain);
            assert(remain >= 0);
            if (likely(remain > 0)) {
                uint64_t data64 = *(uint64_t *)(data);
                size_t rest = (size_t)(kStepSize - remain);
                assert(rest > 0 && rest < (size_t)kStepSize);
                uint64_t mask = kMaskOne >> (rest * 8U);
                data64 &= mask;
                crc64 = _mm_crc32_u64(crc64, data64);
            }
            break;
        }
    } while (1);

    return (uint32_t)~crc64;
#else
    return crc32_x86(data, length);
#endif // CRC32C_IS_X86_64
}

static uint32_t sha1_msg2(const char * data, size_t length)
{
    assert(data != nullptr);
    static const ssize_t kMaxSize = 16;
    static const uint64_t kRestMask = (uint64_t)((kMaxSize / 2) - 1);
    static const uint64_t kRestMask2 = (uint64_t)(kMaxSize - 1);
    static const uint64_t kMaskOne = 0xFFFFFFFFFFFFFFFFULL;

    if (likely(length > 0)) {
        ssize_t remain = (ssize_t)length;

        __m128i __ones = _mm_setzero_si128();
        __m128i __msg1 = _mm_setzero_si128();
        __m128i __msg2 = _mm_setzero_si128();
        __ones = _mm_cmpeq_epi32(__ones, __ones);

        do {
            if (likely(remain <= kMaxSize)) {
                __m128i __data1 = _mm_loadu_si128((const __m128i *)(data + 0));
                __m128i __mask1;

                uint64_t mask1 = kMaskOne;
                uint64_t rest = (uint64_t)(kMaxSize - remain);
                if (likely(rest <= kRestMask)) {
                    mask1 = mask1 >> (rest * 8U);
                    //__mask1 = _mm_cvtsi64_si128(mask1);
                    __mask1 = _mm_set_epi64x(mask1, kMaskOne);
                }
                else if (likely(rest < kMaxSize)) {
                    mask1 = mask1 >> ((rest & kRestMask) * 8U);
                    __mask1 = _mm_set_epi64x(0, mask1);
                }
                else {
                    __mask1 = __ones;
                }

                __data1 = _mm_and_si128(__data1, __mask1);
                __msg1 = _mm_sha1msg2_epu32(__data1, __msg1);

                remain -= kMaxSize;
                break;
            }
            else if (likely(remain <= kMaxSize * 2)) {
                __m128i __data1 = _mm_loadu_si128((const __m128i *)(data + 0));
                __m128i __data2 = _mm_loadu_si128((const __m128i *)(data + kMaxSize));

                __m128i __mask2 = __ones;

                uint64_t rest = (uint64_t)(kMaxSize * 2 - remain);
                if (likely(rest <= kRestMask)) {
                    __m128i __rest = _mm_set_epi64x(0, rest * 8);
                    //__m128i __rest = _mm_cvtsi32_si128((int)(rest * 8));
                    __mask2 = _mm_srl_epi64(__mask2, __rest);
                    __mask2 = _mm_unpackhi_epi64(__ones, __mask2);

                    //__m128d __mask2d = _mm_shuffle_pd(*(__m128d *)&__ones, *(__m128d *)&__mask2, 0b10);
                    //__data2 = _mm_and_si128(__data2, *(__m128i *)&__mask2d);
                }
                else {
                    __m128i __rest = _mm_set_epi64x(0, (rest * 8U - 64));
                    //__m128i __rest = _mm_cvtsi32_si128((int)(rest * 8U - 64));
                    __mask2 = _mm_srl_epi64(__mask2, __rest);
                    __mask2 = _mm_move_epi64(__mask2);
                }

                __data2 = _mm_and_si128(__data2, __mask2);

                __msg1 = _mm_sha1msg2_epu32(__data1, __msg1);
                __msg2 = _mm_sha1msg2_epu32(__data2, __msg2);

                remain -= kMaxSize * 2;
                break;
            }
            else {
                __m128i __data1 = _mm_loadu_si128((const __m128i *)(data + 0));
                __m128i __data2 = _mm_loadu_si128((const __m128i *)(data + kMaxSize));

                __msg1 = _mm_sha1msg2_epu32(__data1, __msg1);
                __msg2 = _mm_sha1msg2_epu32(__data2, __msg2);

                data += kMaxSize * 2;
                remain -= kMaxSize * 2;
            }
        } while (likely(remain > 0));

        __msg1 = _mm_sha1msg1_epu32(__msg1, __msg2);
        __msg1 = _mm_sha1rnds4_epu32(__msg1, __msg2, 3);
        __msg1 = _mm_shuffle_epi32(__msg1, 0x1B);

        uint32_t sha1 = _mm_cvtsi128_si32(__msg1);
        return sha1;
    }

    return 0;
}

#if CRC32C_IS_X86_64

static uint32_t intel_crc32_u64(const char * data, size_t length)
{
    assert(data != nullptr);
    uint64_t crc64 = ~0;

    static const size_t kStepSize = sizeof(uint64_t);
    uint64_t * src = (uint64_t *)data;
    uint64_t * src_end = src + (length / kStepSize);

    while (likely(src < src_end)) {
        crc64 = _mm_crc32_u64(crc64, *src);
        ++src;
    }

    uint32_t crc32 = (uint32_t)crc64;

    size_t i = length / kStepSize * kStepSize;
    while (likely(i < length)) {
        crc32 = _mm_crc32_u8(crc32, data[i]);
        ++i;
    }
    return ~crc32;
}

#endif // CRC32C_IS_X86_64

#if CRC32C_IS_X86_64

static uint32_t intel_crc32_u64_v2(const char * data, size_t length)
{
    assert(data != nullptr);
    uint64_t crc64 = ~0;

    static const size_t kStepSize = sizeof(uint64_t);
    uint64_t * src = (uint64_t *)data;
    uint64_t * src_end = src + (length / kStepSize);

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
    return ~crc32;
}

#endif // CRC32C_IS_X86_64

static uint32_t intel_crc32_u32(const char * data, size_t length)
{
    assert(data != nullptr);
    uint32_t crc32 = ~0;

    static const size_t kStepSize = sizeof(uint32_t);
    uint32_t * src = (uint32_t *)data;
    uint32_t * src_end = src + (length / kStepSize);

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
    return ~crc32;
}

} // namespace jimi

#endif // JIMI_HTTP_CRC32C_H
