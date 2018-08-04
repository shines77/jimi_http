
#ifndef JSTD_NOTHROW_NEW_H
#define JSTD_NOTHROW_NEW_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <new>

#include "jimi/jstd/nothrow_deleter.h"

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

#endif // JSTD_USE_NOTHROW_NEW

#endif // JSTD_NOTHROW_NEW_H
