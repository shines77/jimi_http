
#ifndef JSTD_NOTHROW_NEW_H
#define JSTD_NOTHROW_NEW_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <new>

//
// new_likely() & new_unlikely()
//
#if (defined(__GNUC__) && ((__GNUC__ == 2 && __GNUC_MINOR__ >= 96) || (__GNUC__ >= 3))) \
 || (defined(__clang__) && ((__clang_major__ == 2 && __clang_minor__ >= 1) || (__clang_major__ >= 3)))
// Since gcc v2.96 or clang v2.1
#ifndef new_likely
#define new_likely(expr)        __builtin_expect(!!(expr), 1)
#endif
#ifndef new_unlikely
#define new_unlikely(expr)      __builtin_expect(!!(expr), 0)
#endif
#else // !new_likely() & new_unlikely()
#ifndef new_likely
#define new_likely(expr)        (expr)
#endif
#ifndef new_unlikely
#define new_unlikely(expr)      (expr)
#endif
#endif // new_likely() & new_unlikely()

#if JSTD_USE_NOTHROW_NEW

//
// Normal new (nothrow)
//

#define JSTD_NEW(new_type) \
    new (std::nothrow) new_type

#define JSTD_FREE(pointer) \
    jstd::nothrow_deleter::free(pointer)

#define JSTD_DELETE(pointer) \
    jstd::nothrow_deleter::destroy(pointer)

#define JSTD_NEW_ARRAY(new_type, new_size) \
    new (std::nothrow) new_type[new_size]

#define JSTD_FREE_ARRAY(pointer) \
    jstd::nothrow_deleter::free(pointer)

#define JSTD_DELETE_ARRAY(pointer) \
    jstd::nothrow_deleter::destroy(pointer)

//
// Operator new (nothrow)
//

#define JSTD_OPERATOR_NEW(new_type, new_size) \
    (new_type *)operator new(sizeof(new_type) * (new_size), std::nothrow)

#define JSTD_OPERATOR_FREE(pointer) \
    jstd::nothrow_deleter::free(pointer)

#define JSTD_OPERATOR_DELETE(pointer) \
    jstd::nothrow_deleter::destroy(pointer)

//
// Placement new (nothrow)
//

#define JSTD_PLACEMENT_NEW(new_type, new_size) \
    (new_type *)operator new(sizeof(new_type) * (new_size), \
                            (void *)JSTD_OPERATOR_NEW(new_type, new_size))

#define JSTD_PLACEMENT_FREE(pointer) \
    JSTD_OPERATOR_FREE(pointer)

#define JSTD_PLACEMENT_DELETE(pointer) \
    JSTD_OPERATOR_DELETE(pointer)

//
// Placement new (purely)
//

#define JSTD_PLACEMENT_NEW_EX(new_type, new_size, memory_ptr) \
    (new_type *)operator new(sizeof(new_type) * (new_size), (void *)(memory_ptr))

#define JSTD_PLACEMENT_FREE_EX(pointer) \
    jstd::placement_deleter::free(pointer)

#define JSTD_PLACEMENT_DELETE_EX(pointer) \
    jstd::placement_deleter::destroy(pointer)

//
// if likely
//
#define IF_LIKELY(expr)         if (new_likely(expr))
#define IF_UNLIKELY(expr)       if (new_unlikely(expr))

#else // !JSTD_USE_NOTHROW_NEW

//
// Normal new
//

#define JSTD_NEW(new_type) \
    new new_type

#define JSTD_FREE(pointer) \
    delete pointer

#define JSTD_DELETE(pointer) \
    delete pointer

#define JSTD_NEW_ARRAY(new_type, new_size) \
    new new_type[new_size]

#define JSTD_FREE_ARRAY(pointer) \
    delete[] pointer

#define JSTD_DELETE_ARRAY(pointer) \
    delete[] pointer

//
// Operator new
//

#define JSTD_OPERATOR_NEW(new_type, new_size) \
    (new_type *)operator new(sizeof(new_type) * (new_size))

#define JSTD_OPERATOR_FREE(pointer) \
    operator delete((void *)(pointer))

#define JSTD_OPERATOR_DELETE(pointer) \
    operator delete((void *)(pointer))

//
// Placement new
//

#define JSTD_PLACEMENT_NEW(new_type, new_size) \
    (new_type *)operator new(sizeof(new_type) * (new_size), \
                            (void *)JSTD_OPERATOR_NEW(new_type, new_size))

#define JSTD_PLACEMENT_FREE(pointer) \
    JSTD_OPERATOR_FREE(pointer)

#define JSTD_PLACEMENT_DELETE(pointer) \
    JSTD_OPERATOR_DELETE(pointer)

//
// Placement new (purely)
//

#define JSTD_PLACEMENT_NEW_EX(new_type, new_size, memory_ptr) \
    (new_type *)operator new(sizeof(new_type) * (new_size), (void *)(memory_ptr))

#define JSTD_PLACEMENT_FREE_EX(pointer) \
    operator delete((void *)(pointer), (void *)(pointer))

#define JSTD_PLACEMENT_DELETE_EX(pointer) \
    jstd::placement_deleter::destroy(pointer)

//
// if likely
//
#define IF_LIKELY(expr)
#define IF_UNLIKELY(expr)

#endif // JSTD_USE_NOTHROW_NEW

#include "jimi/jstd/nothrow_deleter.h"

#endif // JSTD_NOTHROW_NEW_H
