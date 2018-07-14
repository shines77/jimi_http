
#ifndef JSTD_STRING_UTILS_H
#define JSTD_STRING_UTILS_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "jimi/basic/stddef.h"
#include "jimi/basic/stdint.h"

#include <nmmintrin.h>  // For SSE 4.2

#include "jimi/support/SSEHelper.h"

namespace jstd {
namespace detail {

template <typename CharTy>
static inline
bool string_equal(const CharTy * str1, const CharTy * str2, size_t length)
{
    assert(str1 != nullptr && str2 != nullptr);

    static const int kMaxSize = jimi::SSEHelper<CharTy>::kMaxSize;
    static const int _SIDD_CHAR_OPS = jimi::SSEHelper<CharTy>::_SIDD_CHAR_OPS;
    static const int kEqualEach = _SIDD_CHAR_OPS | _SIDD_CMP_EQUAL_EACH
                                | _SIDD_NEGATIVE_POLARITY | _SIDD_LEAST_SIGNIFICANT;

    if (likely(length > 0)) {
        ssize_t nlength = (ssize_t)length;
        do {
            __m128i __str1 = _mm_loadu_si128((const __m128i *)str1);
            __m128i __str2 = _mm_loadu_si128((const __m128i *)str2);
            int len = ((int)nlength >= kMaxSize) ? kMaxSize : (int)nlength;
            assert(len > 0);

            int full_matched = _mm_cmpestrc(__str1, len, __str2, len, kEqualEach);
            if (likely(full_matched == 0)) {
                // Full matched, continue match next kMaxSize bytes.
                str1 += kMaxSize;
                str2 += kMaxSize;
                nlength -= kMaxSize;
            }
            else {
                // It's dismatched.
                return false;
            }
        } while (nlength > 0);
    }

    // It's matched, or the length is equal 0, .
    return true;
}

template <typename CharTy>
static inline
bool string_equal_v2(const CharTy * str1, const CharTy * str2, size_t length)
{
    assert(str1 != nullptr && str2 != nullptr);

    static const int kMaxSize = jimi::SSEHelper<CharTy>::kMaxSize;
    static const int _SIDD_CHAR_OPS = jimi::SSEHelper<CharTy>::_SIDD_CHAR_OPS;
    static const int kEqualEach = _SIDD_CHAR_OPS | _SIDD_CMP_EQUAL_EACH
                                | _SIDD_NEGATIVE_POLARITY | _SIDD_LEAST_SIGNIFICANT;

    ssize_t nlength = (ssize_t)length;
    while (likely(nlength > 0)) {
        __m128i __str1 = _mm_loadu_si128((const __m128i *)str1);
        __m128i __str2 = _mm_loadu_si128((const __m128i *)str2);
        int len = ((int)nlength >= kMaxSize) ? kMaxSize : (int)nlength;
        assert(len > 0);

        int full_matched = _mm_cmpestrc(__str1, len, __str2, len, kEqualEach);
        str1 += kMaxSize;
        str2 += kMaxSize;
        nlength -= kMaxSize;
        if (likely(full_matched == 0)) {
            // Full matched, continue match next kMaxSize bytes.
            continue;
        }
        else {
            // It's dismatched.
            return false;
        }
    }

    // It's matched, or the length is equal 0, .
    return true;
}

} // namespace detail
} // namespace jstd

#endif // JSTD_STRING_UTILS_H
