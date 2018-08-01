
#ifndef JIMI_SUPPORT_POWER2_H
#define JIMI_SUPPORT_POWER2_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "jimi/basic/stddef.h"
#include "jimi/basic/stdint.h"

#include "jimi/support/bitscan_reverse.h"
#include "jimi/support/bitscan_forward.h"

namespace jimi {
namespace detail {

static inline
std::size_t round_to_pow2(std::size_t n)
{
    assert(n >= 1);
    if (likely((n & (n - 1)) == 0)) return n;

    unsigned long index;
#if defined(WIN64) || defined(_WIN64) || defined(_M_X64) || defined(_M_AMD64) \
 || defined(_M_IA64) || defined(__amd64__) || defined(__x86_64__) || defined(_M_ARM64)
    unsigned char nonZero = __BitScanReverse64(index, n);
    return (nonZero ? (std::size_t(1) << index) : 1ULL);
#else
    unsigned char nonZero = __BitScanReverse(index, n);
    return (nonZero ? (std::size_t(1) << index) : 1UL);
#endif
}

static inline
std::size_t round_up_pow2(std::size_t n)
{
    assert(n >= 1);
    if (likely((n & (n - 1)) == 0)) return n;

    unsigned long index;
#if defined(WIN64) || defined(_WIN64) || defined(_M_X64) || defined(_M_AMD64) \
 || defined(_M_IA64) || defined(__amd64__) || defined(__x86_64__) || defined(_M_ARM64)
    unsigned char nonZero = __BitScanReverse64(index, n - 1);
    return (nonZero ? (std::size_t(1) << (index + 1)) : 2ULL);
#else
    unsigned char nonZero = __BitScanReverse(index, n - 1);
    return (nonZero ? (std::size_t(1) << (index + 1)) : 2UL);
#endif
}

} // namespace detail
} // namespace jimi

#endif // JIMI_SUPPORT_POWER2_H
