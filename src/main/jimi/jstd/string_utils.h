
#ifndef JSTD_STRING_UTILS_H
#define JSTD_STRING_UTILS_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "jimi/basic/stddef.h"
#include "jimi/basic/stdint.h"

#include <nmmintrin.h>  // For SSE 4.2

#include "jimi/jstd/char_traits.h"
#include "jimi/support/SSEHelper.h"

namespace jstd {
namespace StrUtils {

enum CompareResult {
    IsSmaller = -1,
    IsEqual = 0,
    IsBigger = 1
};

#if 0

template <typename CharTy>
static inline
bool is_equal_unsafe(const CharTy * str1, const CharTy * str2, size_t length)
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

    // It's matched, or the length is equal 0.
    return true;
}

#else

template <typename CharTy>
static inline
bool is_equal_unsafe(const CharTy * str1, const CharTy * str2, size_t length)
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

    // It's matched, or the length is equal 0.
    return true;
}

#endif

template <typename CharTy>
static inline
bool is_equal_fast(const CharTy * str1, const CharTy * str2, size_t length)
{
    if (likely(((uintptr_t)str1 & (uintptr_t)str2) != 0)) {
        assert(str1 != nullptr && str2 != nullptr);
        return StrUtils::is_equal_unsafe(str1, str2, length);
    }
    else if (likely(((uintptr_t)str1 | (uintptr_t)str2) != 0)) {
        assert((str1 == nullptr && str2 != nullptr) ||
               (str1 != nullptr && str2 == nullptr));
        return (length == 0);
    }
    else {
        assert(str1 == nullptr && str2 == nullptr);
        return true;
    }
}

template <typename CharTy>
static inline
bool is_equal(const CharTy * str1, size_t length1, const CharTy * str2, size_t length2)
{
    if (likely(length1 == length2)) {
        return StrUtils::is_equal_fast(str1, str2, length1);
    }

    // The length of between str1 and str2 is different.
    return false;
}

template <typename StringType>
static inline
bool is_equal_fast(const StringType & str1, const StringType & str2)
{
    assert(str1.size() == str2.size());
    return StrUtils::is_equal_fast(str1.c_str(), str2.c_str(), str1.size());
}

template <typename StringType>
static inline
bool is_equal(const StringType & str1, const StringType & str2)
{
    return StrUtils::is_equal(str1.c_str(), str1.size(), str2.c_str(), str2.size());
}

template <typename CharTy>
static inline
int compare_unsafe(const CharTy * str1, size_t length1, const CharTy * str2, size_t length2)
{
    assert(str1 != nullptr && str2 != nullptr);

    static const int kMaxSize = jimi::SSEHelper<CharTy>::kMaxSize;
    static const int _SIDD_CHAR_OPS = jimi::SSEHelper<CharTy>::_SIDD_CHAR_OPS;
    static const int kEqualEach = _SIDD_CHAR_OPS | _SIDD_CMP_EQUAL_EACH
                                | _SIDD_NEGATIVE_POLARITY | _SIDD_LEAST_SIGNIFICANT;

    typedef typename jstd::uchar_traits<CharTy>::type UCharTy;

    size_t length = (length1 <= length2) ? length1 : length2;
    ssize_t nlength = (ssize_t)length;

    while (likely(nlength > 0)) {
        __m128i __str1 = _mm_loadu_si128((const __m128i *)str1);
        __m128i __str2 = _mm_loadu_si128((const __m128i *)str2);
        int len = ((int)nlength >= kMaxSize) ? kMaxSize : (int)nlength;
        assert(len > 0);

        int full_matched = _mm_cmpestrc(__str1, len, __str2, len, kEqualEach);
        int matched_index = _mm_cmpestri(__str1, len, __str2, len, kEqualEach);
        str1 += kMaxSize;
        str2 += kMaxSize;
        nlength -= kMaxSize;
        if (likely(full_matched == 0)) {
            // Full matched, continue match next kMaxSize bytes.
            assert(matched_index >= kMaxSize);
            continue;
        }
        else {
            // It's dismatched.
            assert(matched_index >= 0 && matched_index < kMaxSize);
            int offset = (kMaxSize - matched_index);
            UCharTy ch1 = *((const UCharTy *)str1 - offset);
            UCharTy ch2 = *((const UCharTy *)str2 - offset);
            if (likely(ch1 > ch2))
                return StrUtils::IsBigger;
            else if (likely(ch1 < ch2))
                return StrUtils::IsSmaller;
            else
                return StrUtils::IsEqual;
        }
    }

    // It's matched, or the length is equal 0.
    UCharTy ch1 = *((const UCharTy *)str1);
    UCharTy ch2 = *((const UCharTy *)str2);
    if (likely(ch1 > ch2))
        return StrUtils::IsBigger;
    else if (likely(ch1 < ch2))
        return StrUtils::IsSmaller;
    else
        return StrUtils::IsEqual;
}

template <typename CharTy>
static inline
bool compare(const CharTy * str1, size_t length1, const CharTy * str2, size_t length2)
{
    if (likely(((uintptr_t)str1 & (uintptr_t)str2) != 0)) {
        assert(str1 != nullptr && str2 != nullptr);
        return StrUtils::compare_unsafe(str1, length1, str2, length2);
    }
    else if (likely(((uintptr_t)str1 | (uintptr_t)str2) != 0)) {
        assert((str1 == nullptr && str2 != nullptr) ||
               (str1 != nullptr && str2 == nullptr));
        if (likely(str1 != nullptr))
            return ((length1 != 0) ? StrUtils::IsBigger : StrUtils::IsEqual);
        else
            return ((length2 != 0) ? StrUtils::IsSmaller : StrUtils::IsEqual);
    }
    else {
        assert(str1 == nullptr && str2 == nullptr);
        return StrUtils::IsEqual;
    }
}

template <typename StringType>
static inline
bool compare(const StringType & str1, const StringType & str2)
{
    return StrUtils::compare(str1.c_str(), str1.size(), str2.c_str(), str2.size());
}

} // namespace StrUtils
} // namespace jstd

#endif // JSTD_STRING_UTILS_H
