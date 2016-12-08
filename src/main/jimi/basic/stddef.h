#ifndef JIMI_BASIC_STDDEF_H
#define JIMI_BASIC_STDDEF_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

//
// C++ compiler macro define
// See: http://www.cnblogs.com/zyl910/archive/2012/08/02/printmacro.html
//

#if defined(__GNUC__) && ((__GNUC__ == 2 && __GNUC_MINOR__ >= 96) || (__GNUC__ >= 3))
// Since gcc 2.96
#ifndef likely
#define likely(x)       __builtin_expect(!!(x), 1)
#endif
#ifndef unlikely
#define unlikely(x)     __builtin_expect(!!(x), 0)
#endif
#else
#ifndef likely
#define likely(x)       (x)
#endif
#ifndef unlikely
#define unlikely(x)     (x)
#endif
#endif // likely() & unlikely()

#if defined(_MSC_VER) || defined(__ICL) || defined(__INTEL_COMPILER)
#ifndef ALIGNED_PREFIX
#define ALIGNED_PREFIX(n)       _declspec(align(n))
#endif
#ifndef ALIGNED_SUFFIX
#define ALIGNED_SUFFIX(n)
#endif
#else
#ifndef ALIGNED_PREFIX
#define ALIGNED_PREFIX(n)
#endif
#ifndef ALIGNED_SUFFIX
#define ALIGNED_SUFFIX(n)       __attribute__((aligned(n)))
#endif
#endif // ALIGNED(n)

#endif // !JIMI_BASIC_STDDEF_H
