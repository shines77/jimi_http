
#ifndef JSTD_NOTHROW_DELETER_H
#define JSTD_NOTHROW_DELETER_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <assert.h>
#include <new>

namespace jstd {

struct nothrow_deleter {
    static void destroy(void * p) {
        assert(p != nullptr);
        operator delete(p, std::nothrow);
    }

    template <typename T>
    static void delete_it(T * p) {
        assert(p != nullptr);
        operator delete((void *)p, std::nothrow);
    }

    template <typename T>
    static void destroy(T * p) {
        assert(p != nullptr);
        p->~T();
        operator delete((void *)p, std::nothrow);
    }

    template <typename T>
    static void safe_destroy(void * p) {
        if (p != nullptr) {
            T * pTarget = static<T *>(p);
            assert(pTarget != nullptr);
            if (pTarget != nullptr)
                pTarget->~T();
            operator delete(p, std::nothrow);
        }
    }

    template <typename T>
    static void safe_delete_it(T * p) {
        if (p != nullptr) {
            operator delete((void *)p, std::nothrow);
        }
    }

    template <typename T>
    static void safe_destroy(T * p) {
        if (p != nullptr) {
            p->~T();
            operator delete((void *)p, std::nothrow);
        }
    }
};

} // namespace jstd

#endif // JSTD_NOTHROW_DELETER_H
